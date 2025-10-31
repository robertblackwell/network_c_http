#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "verify_statistics.h"
#include "verify_getopt.h"
#include "verify_thread_context.h"

#define NBR_PROCESSES 1
#define NBR_THREADS 8
#define NBR_CONNECTIONS_PER_THREAD 3
#define NBR_ROUNDTRIPS_PER_CONNECTION 30
#define MAX_ROUNDTRIPS_PER_THREAD (NBR_CONNECTIONS_PER_THREAD * NBR_ROUNDTRIPS_PER_CONNECTION)
#define MAX_RESPONSE_TIMES (NBR_THREADS * MAX_ROUNDTRIPS_PER_THREAD)

void* verify_client_thread_function(void* data);
int main(int argc, char* argv[])
{

    struct timeval main_time_start = get_time();
    char* host_buf = malloc(200);
    strcpy(host_buf, "127.0.0.1");
    int port;
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
    pthread_t workers[nbr_threads];
    VerifyThreadContext* tctx[nbr_threads];
    for(int t = 0; t < nbr_threads; t++) {
        VerifyThreadContext* ctx = verify_ctx_new(port, t, nbr_roundtrips_per_connection, nbr_connections_per_thread, nbr_threads);
        tctx[t] = ctx;
        pthread_create(&(workers[t]), NULL, verify_client_thread_function, (void*)ctx);
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
    const struct timeval main_end_time = get_time();
    const double main_elapsed = time_diff_ms(main_end_time, main_time_start);
    double av_time = main_elapsed / (nbr_threads * 1.0);
    printf("Total elapsed time %f  threads: %d per connections per thread: %d rountrips per connection: %d\n\n", tot_time, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
    printf("Nbr threads : %d  nbr connections per thread: %d nbr of requests per connection: %d av time %f \n\n", nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection, av_time);
    printf("Response times mean: %f stddev: %f total nbr roundtrips: %d \n", avg, stddev, total_roundtrips);
}
