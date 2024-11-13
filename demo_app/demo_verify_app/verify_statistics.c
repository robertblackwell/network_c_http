//
// Created by robert on 11/10/24.
//
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "verify_statistics.h"

struct ResponseTimesArray_s {
    int size_in_doubles;
    int next_index;
    double total_elapsed_time;
    double* readings_ptr;
};
ResponseTimesArrayRef rta_new(int size_in_doubles)
{
    ResponseTimesArrayRef this = malloc(sizeof(ResponseTimesArray));
    this->size_in_doubles = size_in_doubles;
    this->next_index = 0;
    this->readings_ptr = malloc(size_in_doubles * sizeof(double));
    return this;
}
int rta_length(ResponseTimesArrayRef this)
{
    return this->next_index;
}
void rta_expand(ResponseTimesArrayRef this, int to_nbr_doubles)
{
    assert(to_nbr_doubles > this->size_in_doubles);
    this->readings_ptr = realloc(this->readings_ptr, to_nbr_doubles * sizeof(double));
}
void rta_add(ResponseTimesArrayRef this, double value)
{
    if(this->next_index + 1 >= this->size_in_doubles) {
        rta_expand(this, this->size_in_doubles * 2);
    }
    this->readings_ptr[this->next_index] = value;
    this->next_index++;
}
void rta_append(ResponseTimesArrayRef dest, ResponseTimesArrayRef src)
{
    if((dest->next_index + src->next_index) > dest->size_in_doubles) {
        rta_expand(dest, dest->next_index + src->next_index);
    }
    for(int i = 0; i < src->next_index; i++) {
        dest->readings_ptr[dest->next_index] = src->readings_ptr[i];
        dest->next_index++;
    }
}
double rta_at_index(ResponseTimesArrayRef this, int index)
{
    assert(index < this->next_index);
    return this->readings_ptr[index];
}
double rta_get_elapsed_time(ResponseTimesArrayRef this)
{
    return this->total_elapsed_time;
}
void rta_set_elapsed_time(ResponseTimesArrayRef this, double elapsed_time)
{
    this->total_elapsed_time = elapsed_time;
}
void rta_stat_analyse(ResponseTimesArrayRef all, double* average_p, double* stddev_p, int* total_nbr_roundtrips)
{
    double mean = 0.0;
    double total = 0.0;
    int nbr_response_times = rta_length(all);
    for(int i = 0; i < nbr_response_times; i++) {
        double v = rta_at_index(all, i);
        total = total+v;
    }
    mean = total / (nbr_response_times * 1.0);
    double std_total = 0.0;
    for(int i = 0; i < nbr_response_times; i++) {
        double v = rta_at_index(all, i);
        double x = (v-mean);
        double xx = x * x;
        std_total = std_total + xx;
    }
    double variance = std_total / (nbr_response_times * 1.0);
    double stddev = sqrt(variance);
    *average_p = mean;
    *stddev_p = stddev;
    *total_nbr_roundtrips = nbr_response_times;
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
#if 0
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
#endif