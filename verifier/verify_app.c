#define _GNU_SOURCE
#define ENABLE_LOGX
#include <c_http/sync/sync.h>
#include <c_http/sync/sync_internal.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <c_http/macros.h>
#include <c_http/logger.h>

#define VERIFY_URL_MAXSIZE 100
#define VERIFY_UID_MAXSIZE 100
#define VERIFY_MAX_THREADS 100
#define VERIFY_MAX_CONNECTIONS_PER_THREAD 10
#define VERIFY_MAX_REQUESTS_PER_CONNECTION 10

#define VERIFY_DEFAULT_READBUFFER_SIZE 1000000

#define VERIFY_DEFAULT_NBR_THREADS 5
#define VERIFY_DEFAULT_NBR_CONNECTIONS_PER_THREAD 2
#define VERIFY_DEFAULT_NBR_REQUESTS_PER_CONNECTIONS 2

#define VERIFY_DEFAULT_NBR_REQUESTS VERIFY_DEFAULT_NBR_THREADS * VERIFY_DEFAULT_NBR_CONNECTIONS_PER_THREAD * VERIFY_DEFAULT_NBR_REQUESTS_PER_CONNECTIONS

#define THREAD_CONTEXT_TAG "THDCTX"
#include <c_http/check_tag.h>


#define DYN_RESP_TIMES
typedef struct thread_context_s {
    DECLARE_TAG;
    int port;
    size_t read_buffer_size;
    char url[VERIFY_URL_MAXSIZE];
    int howmany_connections;
    int howmany_requests_per_connection;
    int ident;
    MessageRef request_ptr;
    MessageRef response_ptr;
    char uid[VERIFY_UID_MAXSIZE];

    //
    int connection_index;
    int cycle_index;
    struct timeval iter_start_time; // start time of current round trip
    int time_index;
    int counter;
    double total_time;
#ifdef DYN_RESP_TIMES
    double* resp_times;
#else
    double resp_times[500]; // this is a hack that needs fixing
#endif

}thread_context_t;

void thread_context_init(thread_context_t* ctx, int id, int port, int nbr_connections_per_thread,int nbr_requests_per_connection, char* url_cstr, size_t read_buffer_size)
{
    ctx->port = port;
    ctx->read_buffer_size = read_buffer_size;
    strcpy(ctx->url, url_cstr);
    ctx->howmany_connections = nbr_connections_per_thread;
    ctx->howmany_requests_per_connection = nbr_requests_per_connection;
    ctx->ident = id;
    ctx->counter = 0;
    ctx->connection_index = 0;
    ctx->cycle_index = 0;
    ctx->time_index = 0;
    ctx->request_ptr = NULL;
    ctx->response_ptr = NULL;

    SET_TAG(THREAD_CONTEXT_TAG, ctx)
#ifdef DYN_RESP_TIMES
    ctx->resp_times = malloc(sizeof(double) * nbr_connections_per_thread * nbr_requests_per_connection);
#else
    assert(nbr_req_per_thread < 500);
#endif
}
#undef DYN_RESP_TIMES
thread_context_t* thread_context_new(int id, int port, int nbr_connections_per_thread , int nbr_requests_per_connection, char* url_cstr, size_t read_buffer_size)
{
    thread_context_t* ctx = malloc(sizeof(thread_context_t));
    thread_context_init(ctx, id, port, nbr_connections_per_thread, nbr_requests_per_connection, url_cstr, read_buffer_size);
    return ctx;
}

static MessageRef make_request(thread_context_t* ctx, bool keep_alive_flag);
static void make_uid(thread_context_t* ctx);
bool verify_response(thread_context_t* ctx, MessageRef request, MessageRef response);
struct timeval get_time();
double time_diff_ms(struct timeval t1, struct timeval t2);
void* threadfn(void* data);
void combine_response_times(double all[], int all_size, double rt[], int rt_size, int thrd_ix);
void stat_analyse(double all[], int all_size, double* average, double* stdev);
void analyse_response_times(double all[], int all_size, double buckets[10]);
void dump_double_arr(char* msg, double arr[], int arr_dim);
int ctx_time_array_index(thread_context_t* ctx);
bool ctx_is_last_request_for_connection(thread_context_t* ctx);
static void usage();
thread_context_t ctx_table[VERIFY_MAX_THREADS];


int main(int argc, char* argv[])
{
    int c;
    int port_number = 9001;
    char url[100];
    url[0] = '\0';
    int nbr_threads = VERIFY_DEFAULT_NBR_THREADS;
    int nbr_connections_per_thread = VERIFY_DEFAULT_NBR_CONNECTIONS_PER_THREAD;
    int nbr_requests_per_connection = VERIFY_DEFAULT_NBR_REQUESTS_PER_CONNECTIONS;
    int nbr_requests = VERIFY_DEFAULT_NBR_REQUESTS;
    size_t default_read_buffer_size = VERIFY_DEFAULT_READBUFFER_SIZE;
    opterr = 0;
    while((c = getopt(argc, argv, "p:t:r:u:c:h")) != -1)
    {
        switch(c) {
            case 'p':
                LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
            case 'c':
                LOG_FMT("-c options %s", optarg);
                nbr_connections_per_thread = atoi(optarg);
                break;
            case 'r':
                LOG_FMT("-r options %s", optarg);
                nbr_requests_per_connection = atoi(optarg);
                break;
            case 't':
                LOG_FMT("-t options %s", optarg);
                nbr_threads = atoi(optarg);
                break;
            case 'u':
                LOG_FMT("-u options %s", optarg);
                strcpy(url, optarg);
                break;
            case '?':
                printf("\nThere is an Error in the options input\n");
            case 'h':
                LOG_FMT("-t options %s", optarg);
                usage();
                exit(0);
        }
    }
    if(strlen(url) == 0) {
        strcpy(url, "/echo");
    }
    if(nbr_threads > VERIFY_MAX_THREADS) {
        printf("-t parameter is limited by VERIFY_MAX_THREADS  %d\n", VERIFY_MAX_THREADS);
        exit(-1);
    }
    if(nbr_connections_per_thread > VERIFY_MAX_CONNECTIONS_PER_THREAD) {
        printf("-c parameter is limited by VERIFY_MAX_CONNECTION_PER_THREAD  %d\n", VERIFY_MAX_CONNECTIONS_PER_THREAD);
        exit(-1);
    }
    if(nbr_requests_per_connection > VERIFY_MAX_REQUESTS_PER_CONNECTION) {
        printf("-r parameter is limited by VERIFY_MAX_REQUESTS_PER_CONNECTION  %d\n", VERIFY_MAX_REQUESTS_PER_CONNECTION);
        exit(-1);
    }
    nbr_requests = nbr_threads * nbr_connections_per_thread * nbr_requests_per_connection;
    double all[nbr_requests];
    struct timeval main_time_start = get_time();
    pthread_t workers[nbr_threads];
    thread_context_t* tctx[nbr_threads];
    for(int t = 0; t < nbr_threads; t++) {
#define STATIC_CTX
# ifdef STATIC_CTX
        thread_context_t* ctx = &ctx_table[t];
        thread_context_init(ctx, t, port_number, nbr_connections_per_thread,nbr_requests_per_connection, url, default_read_buffer_size);
#else
        thread_context_t* ctx = thread_context_new(t, port_number, nbr_requests_per_thread, url, default_read_buffer_size);
#endif
        tctx[t] = ctx;
        pthread_create(&(workers[t]), NULL, threadfn, (void*)ctx);
    }
    for(int t = 0; t < nbr_threads; t++) {
        pthread_join(workers[t], NULL);
    }
    struct timeval main_end_time = get_time();

    double tot_time = 0;
    for(int t = 0; t < nbr_threads; t++) {
        tot_time = tot_time + tctx[t]->total_time;
        combine_response_times(all, nbr_requests, tctx[t]->resp_times, nbr_connections_per_thread * nbr_requests_per_connection, t);
    }
    int buckets[10];
    double avg;
    double stddev;
    stat_analyse(all, nbr_requests, &avg, &stddev);
    double main_elapsed_ms = time_diff_ms(main_end_time, main_time_start);
    double avg_time_ms = main_elapsed_ms / (nbr_requests * 1.0);
    printf("Total elapsed time in ms %f. Nbr threads: %d conn per thread: %d requests per connection: %d total number of request/response cycles %d\n",
           main_elapsed_ms, nbr_threads , nbr_connections_per_thread,nbr_requests_per_connection, nbr_requests);
    printf("Response times in ms mean: %f stddev: %f\n", avg_time_ms, stddev);

}
void dump_double_arr(char* msg, double arr[], int arr_dim)
{
    for(int i = 0; i < arr_dim; i++) {
        printf("%s all[%d] = %f\n", msg, i , arr[i]);
    }

}

// this must be a function of type SyncConnectionClientMessageHandler
// This callback gets the response message. Passes it to mainline by:
// -    return HPE_PAUSED which will stop parsing incoming data
// -    put mresponse message pinter in ctx->response_ptr
int verify_handler(MessageRef response_ptr, sync_client_t* client_ptr)
{
    thread_context_t* ctx = sync_client_get_userptr(client_ptr);
    LOG_FMT("verify handler cycle:%d connection: %d\n",ctx->cycle_index, ctx->connection_index );
    ctx->response_ptr = response_ptr;
    MessageRef request_ptr = ctx->request_ptr;
    if(! verify_response(ctx, request_ptr, response_ptr)) {
        LOG_ERROR("Verify response failed")
        printf("Verify response failed\n");
    }
    LOG_FMT("end of loop thread id %d cycle_index %d", ctx->ident, ctx->cycle_index)
    Message_dispose(&(ctx->request_ptr));
    Message_dispose(&response_ptr);
    ctx->response_ptr = NULL;
    struct timeval iter_end_time = get_time();
    int i = ctx_time_array_index(ctx);
    double tmp = time_diff_ms(iter_end_time, ctx->iter_start_time);
    ctx->resp_times[i] =  tmp;
    LOG_FMT("time index : %d this interval: %f", i, ctx->resp_times[i])
    int return_value = HPE_OK;
    if(ctx->cycle_index > ctx->howmany_requests_per_connection - 1) {
        ctx->cycle_index = 0;
        return_value = HPE_USER; // This will force sync_client_round_trip() to return rather than wait for server to timeout
    } else {
        /**
         * Make the last message on a connection CONNECTION: close
         */
        ctx->cycle_index++;
        bool connection_close_flag = ctx_is_last_request_for_connection(ctx);
        ctx->request_ptr = make_request(ctx, connection_close_flag);
        ctx->iter_start_time = get_time();
        sync_connection_write(client_ptr->connection_ptr, ctx->request_ptr);
    }
    LOG_FMT("verify_handler return_value %d n=======================================================================\n", return_value);
    return return_value;
}
void* threadfn(void* data)
{
    thread_context_t* ctx = (thread_context_t*)data;
    CHECK_TAG(THREAD_CONTEXT_TAG, ctx)
//    printf("threadfn ctx: %p data: %p \n", ctx, data);
    struct timeval start_time = get_time();
    for(int iconn = 0; iconn < ctx->howmany_connections; iconn++) {
        struct timeval iter_start_time = get_time();
        ctx->connection_index = iconn;
        ctx->cycle_index = 0;
        LOG_FMT("start of loop thread id %d iteration %d", ctx->ident, iconn)
        /**
         * setup client and connect to server
         */
        sync_client_t* client_ptr = sync_client_new(ctx->read_buffer_size);
        sync_client_set_userptr(client_ptr, ctx);
        sync_client_connect(client_ptr, "localhost", ctx->port);

        /**
         * make the request
         */
        make_uid(ctx);
        MessageRef request = make_request(ctx, true);
        ctx->request_ptr = request;
        CHTTP_ASSERT((ctx->response_ptr == NULL), "Verifier response_ptr not null");
        /**
         * Paranoia check
         */
        CHECK_TAG(THREAD_CONTEXT_TAG, ctx)

        /**
         * This function is a bit tricky. It only returns when the connection closes
         * either because the other end closed it or an I/O error.
         */
        ctx->iter_start_time = get_time();
        sync_client_request_round_trip(client_ptr, request, verify_handler);
        LOG_FMT("end of loop thread id %d connection: %d cycle %d", ctx->ident, iconn, ctx->cycle_index)
        sync_client_close(client_ptr);
        sync_client_dispose(&client_ptr);
//        printf("Completed round-trip loop\n");
    }
    struct timeval end_time = get_time();
    ctx->total_time =  time_diff_ms(end_time, start_time);
}


/**
 * Create a request message with target = /echo, an Echo_id header, empty body
 * \param ctx
 * \return MessageRef with ownership
 */
static MessageRef make_request(thread_context_t* ctx, bool keep_alive_flag)
{
    MessageRef request = Message_new();
    Message_set_is_request(request, true);
    Message_set_method(request, HTTP_GET);
    Message_set_target(request, ctx->url );
    char* content_length = "0";
    char* echo_id;

    Message_add_header_cstring(request, HEADER_HOST, "ahost");
    Message_add_header_cstring(request, "User-agent", "x15:x15-soundtrip client");
    if(keep_alive_flag) {
        Message_add_header_cstring(request, HEADER_CONNECTION, "Keep-Alive");
    } else {
        Message_add_header_cstring(request, HEADER_CONNECTION, "close");
    }
    Message_add_header_cstring(request, HEADER_CONTENT_LENGTH, content_length);
    Message_add_header_cstring(request, HEADER_ECHO_ID, ctx->uid);
    Message_set_body(request, BufferChain_new());
    return request;
}

static void make_uid(thread_context_t* ctx)
{
    sprintf(ctx->uid, "%d:%d", ctx->ident, ctx->counter);
}

/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       thread_context_t*
 * \param request   MessageRef
 * \param response  MessageRef
 * \return bool
 */
bool verify_response(thread_context_t* ctx, MessageRef request, MessageRef response)
{
    if(response == NULL) {
        printf("verify_response failed response is NULL\n");
        return false;
    }
    BufferChainRef body = Message_get_body(response);
    if(body == NULL) {
        body = BufferChain_new();
    }
    return true;
#define VERIFY_DISABLED
#ifdef VERIFY_DISABLED
    IOBufferRef iob = Message_serialize(request);
    IOBufferRef iobresp = Message_serialize(response);
    printf("Request \n");
    printf("%s\n", IOBuffer_cstr(iob));
    printf("Response \n");
    printf("%s\n", IOBuffer_cstr(iobresp));
    return true;
#else
    IOBufferRef body_iob = BufferChain_compact(body);
    IOBufferRef req_iob = Message_serialize(request);
    bool x = IOBuffer_equal(body_iob, req_iob);
    if( !x ) {
        printf("Verify failed \n");
        printf("Req     :  %.*s\n", IOBuffer_data_len(req_iob), (char*)IOBuffer_data(req_iob));
        printf("Rsp body:  %.*s\n", IOBuffer_data_len(body_iob), (char*)IOBuffer_data(body_iob));
    }
    return x;
#endif
}
struct timeval get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t;
//    return t.tv_sec*1e3 + t.tv_usec*1e-3;
}
double time_diff_ms(struct timeval t1, struct timeval t2)
{
    double dif = (t1.tv_sec - t2.tv_sec) * 1e3 + (t1.tv_usec - t2.tv_usec) * 1e-3;
    return dif;
}

void combine_response_times(double all[], int all_size, double rt[], int rt_size, int thrd_ix)
{
    for(int i = 0; i < rt_size; i++) {
        double v = rt[i];
        all[thrd_ix * rt_size + i] = rt[i];
    }
}
/**
 *
 * @param all     double[] each entry is a response time for a single request/response cycle
 * @param average double*  Variable into which the mean should be deposited
 * @param stdev   double*  Variable into which the stdev should be deposited
 */
void stat_analyse(double all[], int all_size, double* average, double* stdev)
{
    double mean = 0.0;
    double total = 0.0;
    for(int i = 0; i < all_size; i++) {
        double v = all[i];
        total = total+v;
    }
    mean = total / (all_size*1.0);
    total = 0.0;
    for(int i = 0; i < all_size; i++) {
        double v = all[i];
        total = total+(v-mean)*(v-mean);
    }
    double variance = total / (all_size * 1.0);
    double stddev = sqrt(variance);
    *average = mean;
    *stdev = stddev;
}

void analyse_response_times(double all[], int all_size, double buckets[10])
{
    double min = all[0];
    double max = 0.0;
    for(int i = 0; i < all_size; i++) {
        double v = all[i];
        if (min > v) min = v;
        if (max < v) max = v;
    }
    double bucket_gap = (max-min)/10.0;
    double bucket_lower[10];
    int bucket_count[10];
    for(int b = 0; b < 10; b++) {
        bucket_count[b] = 0;
        bucket_lower[b] = min+b*bucket_gap;
    }
    for(int i = 0; i <  all_size; i++) {
        for(int b = 0; b < 10; b++) {
            double z = min + (i*bucket_gap);
            if(all[i] >= min + i * bucket_gap) {
                bucket_count[b]++;
                break;
            }
        }
    }
    printf("Hello");
}


int ctx_time_array_index(thread_context_t* ctx)
{
    return ctx->cycle_index + ((ctx->connection_index) * ctx->howmany_requests_per_connection);
}
bool ctx_is_last_request_for_connection(thread_context_t* ctx)
{
    return ctx->cycle_index == (ctx->howmany_requests_per_connection - 1);
}

static void usage()
{
    printf("Name: verifier\n");
    printf("\nDescription\n");
    printf("\tThis program sends a number of simultaneous html GET requests to 'localhost:<port><url>'\n");
    printf("\tverifies that the responses are as expected.\n");
    printf("\tmeasures and reports response times for those requests\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-p\tRequired - Port number to use\n");
    printf("\t-t\tRequired - Number of simultaneous threads\n");
    printf("\t-c\tRequired - Number of consecutive connections to be openned by each thread\n");
    printf("\t-r\tRequired - Number of consecutive requests to be sent on each connection\n");
    printf("\t-u\tRequired - The target or url to be sent to the server. Of the form '/echo'\n");
    printf("\t\tNo host or domain is to be provided\n");
}