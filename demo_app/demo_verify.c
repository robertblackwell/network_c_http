#include <http_in_c/demo_protocol/demo_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define NBR_PROCCES 1
#define NBR_THREADS 8
#define NBR_CONNECTIONS_PER_THREAD 3
#define NBR_ROUNDTRIPS_PER_CONNECTION 30
#define MAX_ROUNDTRIPS_PER_THREAD (NBR_CONNECTIONS_PER_THREAD * NBR_ROUNDTRIPS_PER_CONNECTION)
#define MAX_RESPONSE_TIMES (NBR_THREADS * MAX_ROUNDTRIPS_PER_THREAD)

long nbr_round_trips_per_thread = NBR_CONNECTIONS_PER_THREAD * NBR_ROUNDTRIPS_PER_CONNECTION;
long nbr_round_trips = NBR_THREADS * MAX_ROUNDTRIPS_PER_THREAD;


typedef struct ThreadContext {
    /**
     * How many round trips in this experiment
     */
    int max_rountrips_per_connection;
    int rountrips_count;

    int max_connections_per_thread;
    /**
     * Unique identifier for this thread
     */
    int ident;
    /**
     * count of roundtrips completed
     */
    int roundtrip_per_connection_counter;
    int total_roundtrips;
    /**
     * Total time for this thread
     */
    double total_time;
    /**
     * response time for each round trip executed by this thread
     */
    double resp_times[MAX_ROUNDTRIPS_PER_THREAD];
    /**
     * The most recent unique id string generated for round trip
     */
    char uid[100];
}ThreadContext;

ThreadContext* Ctx_new(int id, int max_roundtrips, int max_connections_per_thread, int max_threads)
{
    ThreadContext* ctx = malloc(sizeof(ThreadContext));
    ctx->max_rountrips_per_connection = max_roundtrips;
    ctx->max_connections_per_thread = max_connections_per_thread;
    ctx->ident = id;
    ctx->roundtrip_per_connection_counter = 0;
    ctx->total_roundtrips = 0;
}
/**
 * Create a request message with target = /echo, an Echo_id header, empty body
 * \param ctx
 * \return DemoMessageRef with ownership
 */
DemoMessageRef mk_request(ThreadContext* ctx)
{
    DemoMessageRef request = demo_message_new();
    demo_message_set_is_request(request, true);
    BufferChainRef body = BufferChain_new();
    char buf[100];
    sprintf(buf, "%d %d 1234567890", ctx->ident, ctx->roundtrip_per_connection_counter);
    BufferChain_append_cstr(body, buf);
    demo_message_set_body(request, body);
    return request;
}

void Ctx_mk_uid(ThreadContext* ctx)
{
    sprintf(ctx->uid, "%d:%d", ctx->ident, ctx->roundtrip_per_connection_counter);
}

/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       ThreadContext*
 * \param request   DemoMessageRef
 * \param response  DemoMessageRef
 * \return bool
 */
bool verify_response(ThreadContext* ctx, DemoMessageRef request, DemoMessageRef response)
{
    BufferChainRef body = demo_message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
    return true;
//    CbufferRef req_bc = Message_serialize(request);
//    int x = strcmp(Cbuffer_cstr(body_bc), Cbuffer_cstr(req_bc));
//    if( x != 0) {
//        printf("Verify failed \n");
//        printf("Req     :  %s\n", Cbuffer_cstr(req_bc));
//        printf("Rsp body:  %s\n", Cbuffer_cstr(body_bc));
//    }
//    return (x == 0);
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
    double dif = (double )(t1.tv_sec - t2.tv_sec) + (double)(t1.tv_usec - t2.tv_usec) * 1e-6;
    return dif;
}
void* threadfn(void* data)
{
    ThreadContext* ctx = (ThreadContext*)data;
    struct timeval start_time = get_time();
    for(int i = 0; i < ctx->max_connections_per_thread; i++) {
        struct timeval iter_start_time = get_time();
        DemoClientRef client = democlient_new();
        democlient_connect(client, "localhost", 9011);
        ctx->roundtrip_per_connection_counter = 0;
        while(1) {
            Ctx_mk_uid(ctx);
            DemoMessageRef request = mk_request(ctx);
            DemoMessageRef response = NULL;
            int rc1 = democlient_write_message(client, request);
            if (rc1 != 0) break;
            int rc2 = democlient_read_message(client, &response);
            if (rc2 != 0) break;
            IOBufferRef iob_req = demo_message_serialize(request);
            IOBufferRef iob_resp = demo_message_serialize(response);
            if (!verify_response(ctx, request, response)) {
                printf("Verify response failed");
            }
            struct timeval iter_end_time = get_time();
            ctx->resp_times[ctx->total_roundtrips] =  time_diff_ms(iter_end_time, iter_start_time);
            ctx->roundtrip_per_connection_counter++;
            ctx->total_roundtrips++;
            if(ctx->roundtrip_per_connection_counter >= ctx->max_rountrips_per_connection) {
                break;
            }
            demo_message_dispose(&request);
            if(response != NULL)
                demo_message_dispose(&response);

        }
        democlient_dispose(&client);
    }
    struct timeval end_time = get_time();
    ctx->total_time =  time_diff_ms(end_time, start_time);
    return NULL;
}
/**
 * Append the response times from a single thread onto the array of all response
 * times.
 */
void append_thread_response_times(double all[MAX_RESPONSE_TIMES], const double thread_response_times[MAX_ROUNDTRIPS_PER_THREAD], int thread_ix)
{
//    printf("append_thread_response_times thread_ix: %d\n", thread_ix);
//    for(int i = 0; i < MAX_ROUNDTRIPS_PER_THREAD;i++) {
//        printf("i: %d rt[i]: %f\n", i, thread_response_times[i]);
//    }
    for(int i = 0; i < MAX_ROUNDTRIPS_PER_THREAD; i++) {
        double v = thread_response_times[i];
        all[(thread_ix * MAX_ROUNDTRIPS_PER_THREAD) + i] = thread_response_times[i];
//        printf("i: %d input: %f all[i:%d]: %f\n", i, thread_response_times[i], ((thread_ix * MAX_ROUNDTRIPS_PER_THREAD) + i), all[(thread_ix * MAX_ROUNDTRIPS_PER_THREAD) + i]);
    }
//    for(int i = 0; i < MAX_RESPONSE_TIMES; i++) {
//        printf("i: %d all[i]: %f\n", i, all[i]);
//    }
}
void stat_analyse(const double all[MAX_RESPONSE_TIMES], double* average, double* stdev)
{
    double mean = 0.0;
    double total = 0.0;
    for(int i = 0; i < MAX_RESPONSE_TIMES; i++) {
        double v = all[i];
        total = total+v;
    }
    mean = total / (MAX_RESPONSE_TIMES * 1.0);
    double std_total = 0.0;
    for(int i = 0; i < MAX_RESPONSE_TIMES; i++) {
        double v = all[i];
        double x = (v-mean);
        double xx = x * x;
        std_total = std_total + xx;
    }
    double variance = std_total / (MAX_RESPONSE_TIMES * 1.0);
    double stddev = sqrt(variance);
    *average = mean;
    *stdev = stddev;
}

void analyse_response_times(double all[MAX_RESPONSE_TIMES], double buckets[10])
{
    double min = all[0];
    double max = 0.0;
    for(int i = 0; i < MAX_RESPONSE_TIMES; i++) {
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
    for(int i = 0; i <  MAX_RESPONSE_TIMES; i++) {
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

    double all[MAX_RESPONSE_TIMES];
    struct timeval main_time_start = get_time();
    pthread_t workers[NBR_THREADS];
    ThreadContext* tctx[NBR_THREADS];
    for(int t = 0; t < NBR_THREADS; t++) {
        ThreadContext* ctx = Ctx_new(t, NBR_ROUNDTRIPS_PER_CONNECTION, NBR_CONNECTIONS_PER_THREAD, NBR_THREADS);
        tctx[t] = ctx;
        pthread_create(&(workers[t]), NULL, threadfn, (void*)ctx);
    }
    double tot_time = 0;
    for(int t = 0; t < NBR_THREADS; t++) {
        pthread_join(workers[t], NULL);
        tot_time = tot_time + tctx[t]->total_time;
        append_thread_response_times(all, tctx[t]->resp_times, t);
    }
    int buckets[10];
    double avg;
    double stddev;
    stat_analyse(all, &avg, &stddev);
    struct timeval main_end_time = get_time();
    double main_elapsed = time_diff_ms(main_end_time, main_time_start);
    double av_time = main_elapsed / (NBR_THREADS * 1.0);
    printf("Total elapsed time %f  threads: %d per connections per thread: %d rountrips per connection: %d\n", tot_time, NBR_THREADS, NBR_CONNECTIONS_PER_THREAD, NBR_ROUNDTRIPS_PER_CONNECTION);
    printf("Nbr threads : %d  nbr connections per thread: %d nbr of requests per connection: %d av time %f \n", NBR_THREADS, NBR_CONNECTIONS_PER_THREAD, NBR_ROUNDTRIPS_PER_CONNECTION, av_time);
    printf("Response times mean: %f stddev: %f total nbr roundtrips: %d \n", avg, stddev, MAX_RESPONSE_TIMES);

}

