#include <src/tmpl_protocol/tmpl_sync_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "verify_getopt.h"
#include "verify_statistics.h"
#include "verify_thread_context.h"
#include "../tmpl_common/tmpl_make_request_response.h"


#define NBR_PROCCES 1
#define NBR_THREADS 8
#define NBR_CONNECTIONS_PER_THREAD 3
#define NBR_ROUNDTRIPS_PER_CONNECTION 30
#define MAX_ROUNDTRIPS_PER_THREAD (NBR_CONNECTIONS_PER_THREAD * NBR_ROUNDTRIPS_PER_CONNECTION)
#define MAX_RESPONSE_TIMES (NBR_THREADS * MAX_ROUNDTRIPS_PER_THREAD)

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
    ThreadContext* tctx[nbr_threads];
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
    printf("Total elapsed time %f  threads: %d per connections per thread: %d rountrips per connection: %d\n\n", tot_time, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
    printf("Nbr threads : %d  nbr connections per thread: %d nbr of requests per connection: %d av time %f \n\n", nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection, av_time);
    printf("Response times mean: %f stddev: %f total nbr roundtrips: %d \n", avg, stddev, total_roundtrips);
}
void* threadfn(void* data)
{
    ThreadContext* ctx = (ThreadContext*)data;
    struct timeval start_time = get_time();
    for(int i = 0; i < ctx->max_connections_per_thread; i++) {
        TmplSyncSocketRef client = tmpl_syncsocket_new();
        tmpl_syncsocket_connect(client, "127.0.0.1", 9011);
        ctx->roundtrip_per_connection_counter = 0;
        while(1) {
            struct timeval iter_start_time = get_time();
            Ctx_mk_uid(ctx);
            TmplMessageRef request = tmpl_make_request(ctx);
            TmplMessageRef response = NULL;
            int rc1 = tmpl_syncsocket_write_message(client, request);
            if (rc1 != 0) break;
            int rc2 = tmpl_syncsocket_read_message(client, &response);
            if (rc2 != 0) break;
            IOBufferRef iob_req = tmpl_message_serialize(request);
            IOBufferRef iob_resp = tmpl_message_serialize(response);
            if (!verify_response(ctx, request, response)) {
                printf("Tmpl Verify response failed");
            } else {
                printf("Tmpl verify succeeded\n");
            }
            struct timeval iter_end_time = get_time();

            rta_add(ctx->response_times_ref, time_diff_ms(iter_end_time, iter_start_time));

            ctx->roundtrip_per_connection_counter++;
            ctx->total_roundtrips++;
            if(ctx->roundtrip_per_connection_counter >= ctx->max_rountrips_per_connection) {
                break;
            }
            tmpl_message_free(request);
            request = NULL;
            if(response != NULL) {
                tmpl_message_free(response);
                response = NULL;
            }
        }
        tmpl_syncsocket_free(client);
    }
    struct timeval end_time = get_time();

    rta_set_elapsed_time(ctx->response_times_ref, time_diff_ms(end_time, start_time));
//    ctx->total_time =  time_diff_ms(end_time, start_time);

return NULL;
}
