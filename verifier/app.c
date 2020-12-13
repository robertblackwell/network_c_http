#include <c_http/api/client.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define NBR_CONCURRENT 30
#define NBR_PER_THREAD 100

typedef struct ThreadContext {
    /**
     * How many round trips in this experiment
     */
    int howmany;
    /**
     * Unique identifier for this thread
     */
    int ident;
    /**
     * count of roundtrips completed
     */
    int counter;
    double total_time;
    double resp_times[NBR_PER_THREAD];
    /**
     * The most recent unique id string generated for round trip
     */
    char uid[100];
}ThreadContext;

ThreadContext* Ctx_new(int id)
{
    ThreadContext* ctx = malloc(sizeof(ThreadContext));
    ctx->howmany = NBR_PER_THREAD;
    ctx->ident = id;
    ctx->counter = 0;
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
    HdrListRef request_hdrs = Message_get_headerlist(request);
    char* content_length = "0";
    char* echo_id;

    HdrList_add_cstr(request_hdrs, HEADER_HOST, "ahost");
    HdrList_add_cstr(request_hdrs, "User-agent", "x15:x15-soundtrip client");
    HdrList_add_cstr(request_hdrs, HEADER_CONNECTION, "close");
    HdrList_add_cstr(request_hdrs, HEADER_CONTENT_LENGTH, content_length);
    HdrList_add_cstr(request_hdrs, HEADER_ECHO_ID, ctx->uid);
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

    BufferChainRef body = Message_get_body(response);
    printf("verify_response body: %p\n", body);
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
    return (x == 0);
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
    double dif = (t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) * 1e-6;
    return dif;
}
void* threadfn(void* data)
{
    ThreadContext* ctx = (ThreadContext*)data;
    struct timeval start_time = get_time();
    for(int i = 0; i < ctx->howmany; i++) {
        struct timeval iter_start_time = get_time();
        MessageRef response;
        ClientRef client = Client_new();
        Client_connect(client, "localhost", 9001);
        Ctx_mk_uid(ctx);
        MessageRef request = mk_request(ctx);
        IOBufferRef serialized = Message_serialize(request);
        const char* req_buffer[] = {
            IOBuffer_cstr(serialized), NULL
        };
        for(int i = 0; req_buffer[i] != NULL; i++) {
            const char* x = req_buffer[i];
        }

        Client_roundtrip(client, req_buffer,  &response);
        IOBufferRef cb = Message_serialize(response);

        if(! verify_response(ctx, request, response)) {
            printf("Verify response failed");
        }
        ctx->counter++;
        IOBuffer_free(&serialized);
        Message_free(&request);
        Message_free(&response);
        Client_free(&client);
        struct timeval iter_end_time = get_time();
        ctx->resp_times[i] =  time_diff_ms(iter_end_time, iter_start_time);
    }
    struct timeval end_time = get_time();
    ctx->total_time =  time_diff_ms(end_time, start_time);
}

void combine_response_times(double all[NBR_PER_THREAD*NBR_CONCURRENT], double rt[NBR_PER_THREAD], int thrd_ix)
{
    for(int i = 0; i < NBR_PER_THREAD; i++) {
        double v = rt[i];
        all[thrd_ix*NBR_PER_THREAD + i] = rt[i];
    }
}
void stat_analyse(double all[NBR_PER_THREAD*NBR_CONCURRENT], double* average, double* stdev)
{
    double mean = 0.0;
    double total = 0.0;
    for(int i = 0; i < NBR_PER_THREAD*NBR_CONCURRENT; i++) {
        double v = all[i];
        total = total+v;
    }
    mean = total / (NBR_PER_THREAD*NBR_CONCURRENT*1.0);
    total = 0.0;
    for(int i = 0; i < NBR_PER_THREAD*NBR_CONCURRENT; i++) {
        double v = all[i];
        total = total+(v-mean)*(v-mean);
    }
    double variance = total / (NBR_PER_THREAD*NBR_CONCURRENT*1.0);
    double stddev = sqrt(variance);
    *average = mean;
    *stdev = stddev;
}

void analyse_response_times(double all[NBR_PER_THREAD*NBR_CONCURRENT], double buckets[10])
{
    double min = all[0];
    double max = 0.0;
    for(int i = 0; i < NBR_PER_THREAD*NBR_CONCURRENT; i++) {
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
    for(int i = 0; i <  NBR_PER_THREAD*NBR_CONCURRENT; i++) {
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

int main()
{
    int x1 = sizeof(char);
    int x2 = sizeof(char*);
    int x3 = sizeof(void*);
    int x4 = sizeof(int);
    int x5 = sizeof(long);
    int x6 = sizeof(long long);

    double all[NBR_CONCURRENT*NBR_PER_THREAD];
    struct timeval main_time_start = get_time();
    pthread_t workers[NBR_CONCURRENT];
    ThreadContext* tctx[NBR_CONCURRENT];
    for(int t = 0; t < NBR_CONCURRENT; t++) {
        ThreadContext* ctx = Ctx_new(t);
        tctx[t] = ctx;
        pthread_create(&(workers[t]), NULL, threadfn, (void*)ctx);
    }
    double tot_time = 0;
    for(int t = 0; t < NBR_CONCURRENT; t++) {
        pthread_join(workers[t], NULL);
        tot_time = tot_time + tctx[t]->total_time;
        combine_response_times(all, tctx[t]->resp_times, t);
    }
    int buckets[10];
    double avg;
    double stddev;
    stat_analyse(all, &avg, &stddev);
    struct timeval main_end_time = get_time();
    double main_elapsed = time_diff_ms(main_end_time, main_time_start);
    double av_time = main_elapsed / (NBR_CONCURRENT * 1.0);
    printf("Total elapsed time %f  threads: %d per thread: %d\n", main_elapsed, NBR_CONCURRENT, NBR_PER_THREAD);
    printf("Nbr threads : %d  nbr of requests per thread: %d av time %f \n", NBR_CONCURRENT, NBR_PER_THREAD, av_time);
    printf("Response times mean: %f stddev: %f\n", avg, stddev);

}

