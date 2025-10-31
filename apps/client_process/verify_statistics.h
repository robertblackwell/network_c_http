//
// Created by robert on 11/10/24.
//
#ifndef C_HTTP_VERIFY_STATISTICS_H
#define C_HTTP_VERIFY_STATISTICS_H

#include <time.h>
#include <sys/time.h>
#include <math.h>


typedef struct ResponseTimesArray_s ResponseTimesArray, *ResponseTimesArrayRef;

ResponseTimesArrayRef rta_new(int size_in_doubles);
void rta_start_measurement(ResponseTimesArrayRef this);
void rta_start_round_trip(ResponseTimesArrayRef this);
void rta_end_round_trip(ResponseTimesArrayRef this);
void rta_end_measurement(ResponseTimesArrayRef this);
int rta_length(ResponseTimesArrayRef this);
void rta_add(ResponseTimesArrayRef this, double value);
void rta_append(ResponseTimesArrayRef dest, ResponseTimesArrayRef src);
double rta_at_index(ResponseTimesArrayRef this, int index);
double rta_get_elapsed_time(ResponseTimesArrayRef this);
void rta_set_elapsed_time(ResponseTimesArrayRef this, double elapsed_time);
void rta_stat_analyse(ResponseTimesArrayRef all, double* average, double* stddev, int* total_roundtrips);

struct timeval get_time();
double time_diff_ms(struct timeval t1, struct timeval t2);
//void append_thread_response_times(double all[MAX_RESPONSE_TIMES], const double thread_response_times[MAX_ROUNDTRIPS_PER_THREAD], int thread_ix);
//void stat_analyse(const double all[MAX_RESPONSE_TIMES], double* average, double* stdev);
//void analyse_response_times(double all[MAX_RESPONSE_TIMES], double buckets[10]);

#endif //C_HTTP_VERIFY_STATISTICS_H
