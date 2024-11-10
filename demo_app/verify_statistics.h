//
// Created by robert on 11/10/24.
//
#ifndef C_HTTP_VERIFY_STATISTICS_H
#define C_HTTP_VERIFY_STATISTICS_H

#include <time.h>
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

struct timeval get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t;
//    return t.tv_sec*1e3 + t.tv_usec*1e-3;
}
double time_diff_ms(struct timeval t1, struct timeval t2);
void append_thread_response_times(double all[MAX_RESPONSE_TIMES], const double thread_response_times[MAX_ROUNDTRIPS_PER_THREAD], int thread_ix);
void stat_analyse(const double all[MAX_RESPONSE_TIMES], double* average, double* stdev);
void analyse_response_times(double all[MAX_RESPONSE_TIMES], double buckets[10]);

#endif //C_HTTP_VERIFY_STATISTICS_H
