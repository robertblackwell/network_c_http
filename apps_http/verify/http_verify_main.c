#include <src/http_protocol/http_sync_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "http_verify_getopt.h"
#include "src/common/verify_statistics.h"
#include "http_verify_thread_context.h"
#include "http_common/http_make_request_response.h"


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

    struct timeval main_time_start = get_time();
    char* host_buf = malloc(200);
    strcpy(host_buf, "127.0.0.1");
    int port = 9011;
    int nbr_threads = NBR_THREADS;
    int nbr_connections_per_thread = NBR_CONNECTIONS_PER_THREAD;
    int nbr_roundtrips_per_connection = NBR_ROUNDTRIPS_PER_CONNECTION;
    int total_nbr_roundtrips = nbr_threads * nbr_connections_per_thread * nbr_roundtrips_per_connection;
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
    pthread_t workers[nbr_threads];
    VerifyThreadContext* tctx[nbr_threads];
    for(int t = 0; t < nbr_threads; t++) {
        VerifyThreadContext* ctx = Ctx_new(t, nbr_roundtrips_per_connection, nbr_connections_per_thread, nbr_threads);
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
    VerifyThreadContext* ctx = (VerifyThreadContext*)data;
    struct timeval start_time = get_time();
    for(int i = 0; i < ctx->max_connections_per_thread; i++) {
        HttpSyncSocketRef client = http_syncsocket_new();
        http_syncsocket_connect(client, "localhost", 9011);
        ctx->roundtrip_per_connection_counter = 0;
        while(1) {
            struct timeval iter_start_time = get_time();
            bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;
            HttpMessageRef request = http_make_request("/echo", last_round);
            HttpMessageRef response = NULL;
//            printf("verify about to write message\n");
            int rc1 = http_syncsocket_write_message(client, request);
//            printf("verify write message done\n");
            if (rc1 != 0) break;
//            printf("verify about to read message\n");
            int rc2 = http_syncsocket_read_message(client, &response);
//            printf("verify read message done\n");
            if (rc2 != 0) break;
            IOBufferRef iob_req = http_message_serialize(request);
            IOBufferRef iob_resp = http_message_serialize(response);
            if (!http_verify_response(request, response)) {
                printf("Http Verify response failed\n");
            } else {
                printf("Http verify succeeded\n");
            }
            struct timeval iter_end_time = get_time();

            rta_add(ctx->response_times_ref, time_diff_ms(iter_end_time, iter_start_time));

            ctx->roundtrip_per_connection_counter++;
            ctx->total_roundtrips++;
            if(ctx->roundtrip_per_connection_counter >= ctx->max_rountrips_per_connection) {
                break;
            }
            http_message_free(request);
            request = NULL;
            if(response != NULL) {
                http_message_free(response);
                response = NULL;
            }
        }
        http_syncsocket_free(client);
    }
    struct timeval end_time = get_time();

    rta_set_elapsed_time(ctx->response_times_ref, time_diff_ms(end_time, start_time));
//    ctx->total_time =  time_diff_ms(end_time, start_time);

return NULL;
}
