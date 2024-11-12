#include <http_in_c/demo_protocol/demo_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "verify_getopt.h"
#include "verify_statistics.h"
#include "verify_thread_context.h"


#define NBR_PROCCES 1
#define NBR_THREADS 8
#define NBR_CONNECTIONS_PER_THREAD 3
#define NBR_ROUNDTRIPS_PER_CONNECTION 30
#define MAX_ROUNDTRIPS_PER_THREAD (NBR_CONNECTIONS_PER_THREAD * NBR_ROUNDTRIPS_PER_CONNECTION)
#define MAX_RESPONSE_TIMES (NBR_THREADS * MAX_ROUNDTRIPS_PER_THREAD)

#if 0
#define NBR_PROCCES 1
#define nbr_threads 8
#define nbr_connections_per_thread 3
#define nbr_roundtriips_per_connection 30
#define MAX_ROUNDTRIPS_PER_THREAD (nbr_connections_per_thread * nbr_roundtriips_per_connection)
#define MAX_RESPONSE_TIMES (nbr_threads * MAX_ROUNDTRIPS_PER_THREAD)

long nbr_round_trips_per_thread = nbr_connections_per_thread * nbr_roundtriips_per_connection;
long nbr_round_trips = nbr_threads * MAX_ROUNDTRIPS_PER_THREAD;
#endif
void* threadfn(void* data);

int main(int argc, char* argv[])
{
    int x1 = sizeof(char);
    int x2 = sizeof(char*);
    int x3 = sizeof(void*);
    int x4 = sizeof(int);
    int x5 = sizeof(long);
    int x6 = sizeof(long long);
//    double all[MAX_RESPONSE_TIMES];

    struct timeval main_time_start = get_time();
    char* host_buf = malloc(200);
    strcpy(host_buf, "127.0.0.1");
    int port = 9011;
    int nbr_threads = NBR_THREADS;
    int nbr_connections_per_thread = NBR_CONNECTIONS_PER_THREAD;
    int nbr_roundtrips_per_connection = NBR_ROUNDTRIPS_PER_CONNECTION;
    int total_nbr_roundtrips = nbr_threads * nbr_connections_per_thread * nbr_roundtrips_per_connection;
    pthread_t workers[nbr_threads];
    ThreadContext* tctx[nbr_threads];
    /**
     * get run parameters
     */
    verify_process_args(argc, argv,
                        &host_buf, &port,
                        &nbr_roundtrips_per_connection,
                        &nbr_connections_per_thread,
                        &nbr_threads);
    /**
     * run the test threads
     */
    for(int t = 0; t < nbr_threads; t++) {
        ThreadContext* ctx = Ctx_new(t, nbr_roundtrips_per_connection, nbr_connections_per_thread, nbr_threads);
        tctx[t] = ctx;
        pthread_create(&(workers[t]), NULL, threadfn, (void*)ctx);
    }
    double tot_time = 0;
    /**
     * Consolidate the results from each thread
     */
    ResponseTimesArrayRef all_readings = rta_new(total_nbr_roundtrips);
    for(int t = 0; t < nbr_threads; t++) {
        pthread_join(workers[t], NULL);

        rta_append(all_readings, tctx[t]->response_times_ref);

//        tot_time = tot_time + tctx[t]->total_time;
//        append_thread_response_times(all, tctx[t]->resp_times, t);
    }
    /**
     * Analyse the results
     */
    int buckets[10];
    double avg;
    double stddev;
    int total_roundtrips;
    rta_stat_analyse(all_readings, &avg, &stddev, &total_roundtrips);
    struct timeval main_end_time = get_time();
    double main_elapsed = time_diff_ms(main_end_time, main_time_start);
    double av_time = main_elapsed / (nbr_threads * 1.0);
    printf("Total elapsed time %f  threads: %d per connections per thread: %d rountrips per connection: %d\n", tot_time, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
    printf("Nbr threads : %d  nbr connections per thread: %d nbr of requests per connection: %d av time %f \n", nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection, av_time);
    printf("Response times mean: %f stddev: %f total nbr roundtrips: %d \n", avg, stddev, total_roundtrips);
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

            rta_add(ctx->response_times_ref, time_diff_ms(iter_end_time, iter_start_time));

//            ctx->resp_times[ctx->total_roundtrips] =  time_diff_ms(iter_end_time, iter_start_time);
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

    rta_set_elapsed_time(ctx->response_times_ref, time_diff_ms(end_time, start_time));
//    ctx->total_time =  time_diff_ms(end_time, start_time);

return NULL;
}
