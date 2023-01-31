#include <c_http/sync/sync_client.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#define ENABLE_LOGX
#include <c_http/logger.h>

#define NBR_THREADS 10
#define NBR_REQUESTS_PER_THREAD 10
#define NBR_REQUESTS NBR_THREADS*NBR_REQUESTS_PER_THREAD

#define ThreadCtx_TAG "THDCTX"
#include <c_http/check_tag.h>

#define DYN_RESP_TIMES
typedef struct ThreadContext {
    DECLARE_TAG;
    /**
     * the port to connect to
     */
     int port;
    /**
     * How many round trips in this experiment
     */
    int howmany;
    /**
     * Unique identifier for this thread
     */
    int ident;
    /**
     * The most recent unique id string generated for round trip
     */
    char uid[100];
    /**
     * count of roundtrips completed
     */
    int counter;
    double total_time;
#ifdef DYN_RESP_TIMES
    double* resp_times;
#else
    double resp_times[500]; // this is a hack that needs fixing
#endif

}ThreadContext;

ThreadContext* Ctx_new(int id, int port, int nbr_req_per_thread)
{
    ThreadContext* ctx = malloc(sizeof(ThreadContext));
    ctx->port = port;
    ctx->howmany = nbr_req_per_thread;
    ctx->ident = id;
    ctx->counter = 0;
    SET_TAG(ThreadCtx_TAG, ctx)
#ifdef DYN_RESP_TIMES
    ctx->resp_times = malloc(sizeof(double) * nbr_req_per_thread);
#else
    assert(nbr_req_per_thread < 500);
#endif
    return ctx;
}
#undef DYN_RESP_TIMES

MessageRef mk_request(ThreadContext* ctx);
void Ctx_mk_uid(ThreadContext* ctx);
bool verify_response(ThreadContext* ctx, MessageRef request, MessageRef response);
struct timeval get_time();
double time_diff_ms(struct timeval t1, struct timeval t2);
void* threadfn(void* data);
void combine_response_times(double all[], int all_size, double rt[], int rt_size, int thrd_ix);
void stat_analyse(double all[], int all_size, double* average, double* stdev);
void analyse_response_times(double all[], int all_size, double buckets[10]);
void dump_double_arr(char* msg, double arr[], int arr_dim);

int main(int argc, char* argv[])
{

    int c;
    int port_number = 9001;
    int nbr_threads = NBR_THREADS;
    int nbr_requests_per_thread = NBR_REQUESTS_PER_THREAD;
    int nbr_requests = NBR_REQUESTS;
    while((c = getopt(argc, argv, "p:t:r:")) != -1)
    {
        switch(c) {
            case 'p':
                LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
            case 'r':
                LOG_FMT("-r options %s", optarg);
                nbr_requests_per_thread = atoi(optarg);
                break;
            case 't':
                LOG_FMT("-t options %s", optarg);
                nbr_threads = atoi(optarg);
                break;
        }

    }
    nbr_requests = nbr_threads * nbr_requests_per_thread;
    double all[nbr_requests];
    struct timeval main_time_start = get_time();
    pthread_t workers[nbr_threads];
    ThreadContext* tctx[nbr_threads];
    for(int t = 0; t < nbr_threads; t++) {
        ThreadContext* ctx = Ctx_new(t, port_number, nbr_requests_per_thread);
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
        combine_response_times(all, nbr_requests, tctx[t]->resp_times, nbr_requests_per_thread, t);
    }
    int buckets[10];
    double avg;
    double stddev;
    stat_analyse(all, nbr_requests, &avg, &stddev);
    double main_elapsed_ms = time_diff_ms(main_end_time, main_time_start);
    double av_time_ms = main_elapsed_ms / (nbr_threads * 1.0);
    printf("Total elapsed time in ms %f. Nbr threads: %d requests per thread: %d total number of request/response cycles %d\n", main_elapsed_ms, nbr_threads , nbr_requests_per_thread, nbr_requests);
    printf("Response times in ms mean: %f stddev: %f\n", avg, stddev);

}
void dump_double_arr(char* msg, double arr[], int arr_dim)
{
    for(int i = 0; i < arr_dim; i++) {
        printf("%s all[%d] = %f\n", msg, i , arr[i]);
    }

}
void* threadfn(void* data)
{
    ThreadContext* ctx = (ThreadContext*)data;
    CHECK_TAG(ThreadCtx_TAG,  ctx)
    struct timeval start_time = get_time();
    for(int i = 0; i < ctx->howmany; i++) {
        struct timeval iter_start_time = get_time();
        MessageRef response;
        LOG_FMT("start of loop thread id %d iteration %d", ctx->ident, i)
        ClientRef client = Client_new();
        Client_connect(client, "localhost", ctx->port);
        Ctx_mk_uid(ctx);
        MessageRef request = mk_request(ctx);
        CHECK_TAG(ThreadCtx_TAG,  ctx)
        Client_request_round_trip(client, request, &response);

        if(! verify_response(ctx, request, response)) {
            LOG_ERROR("Verify response failed")
            printf("Verify response failed\n");
        }
        LOG_FMT("end of loop thread id %d iteration %d", ctx->ident, i)
        ctx->counter++;
        Message_dispose(&request);
        Message_dispose(&response);
        Client_dispose(&client);
        struct timeval iter_end_time = get_time();
        ctx->resp_times[i] =  time_diff_ms(iter_end_time, iter_start_time);
    }
    struct timeval end_time = get_time();
    ctx->total_time =  time_diff_ms(end_time, start_time);
}


/**
 * Create a request message with target = /echo, an Echo_id header, empty body
 * \param ctx
 * \return MessageRef with ownership
 */
MessageRef mk_request(ThreadContext* ctx)
{
    MessageRef request = Message_new();
    Message_set_is_request(request, true);
    Message_set_method(request, HTTP_GET);
    Message_set_target(request, "/echo" );
    char* content_length = "0";
    char* echo_id;

    Message_add_header_cstring(request, HEADER_HOST, "ahost");
    Message_add_header_cstring(request, "User-agent", "x15:x15-soundtrip client");
    Message_add_header_cstring(request, HEADER_CONNECTION, "close");
    Message_add_header_cstring(request, HEADER_CONTENT_LENGTH, content_length);
    Message_add_header_cstring(request, HEADER_ECHO_ID, ctx->uid);
    Message_set_body(request, BufferChain_new());
    return request;
}

void Ctx_mk_uid(ThreadContext* ctx)
{
    sprintf(ctx->uid, "%d:%d", ctx->ident, ctx->counter);
}

/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       ThreadContext*
 * \param request   MessageRef
 * \param response  MessageRef
 * \return bool
 */
bool verify_response(ThreadContext* ctx, MessageRef request, MessageRef response)
{
    if(response == NULL) {
        printf("verify_response failed response is NULL\n");
        return false;
    }
    BufferChainRef body = Message_get_body(response);
    if(body == NULL) {
        body = BufferChain_new();
    }
    IOBufferRef body_iob = BufferChain_compact(body);
    IOBufferRef req_iob = Message_serialize(request);
    bool x = IOBuffer_equal(body_iob, req_iob);
    if( !x ) {
        printf("Verify failed \n");
        printf("Req     :  %.*s\n", IOBuffer_data_len(req_iob), (char*)IOBuffer_data(req_iob));
        printf("Rsp body:  %.*s\n", IOBuffer_data_len(body_iob), (char*)IOBuffer_data(body_iob));
    }
    return x;
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

