//
// Created by robert on 11/10/24.
//
#include "response_times.h"
#include <stdlib.h>
#include <string.h>
/**
 * Append the response times from a single thread onto the array of all response
 * times.
 */

typedef struct ResponseTimes_s {
    unsigned long count;
    unsigned long size;
    union { ;
        double times[];
        double* times_ptr;
    };
} ResponseTimes, *ResponseTimesRef;

void response_times_init(ResponseTimesRef rt, unsigned long size)
{
    rt->size = size;
    rt->count = 0;
    rt->times_ptr = malloc(sizeof(double) * size);
}
ResponseTimesRef response_times_new(unsigned long size)
{
    ResponseTimesRef this = malloc(sizeof(ResponseTimes));
    response_times_init(this, size);
    return this;
}
void response_times_free(ResponseTimesRef this)
{
    free(this->times_ptr);
    free(this);
}
void response_times_add(ResponseTimesRef rt, double value)
{
    if(rt->count + 1 == rt->size) {
        double* tmp = malloc(2 * sizeof(double) * rt->size);
        memcpy(tmp, rt->times_ptr, rt->size);
        rt->size = 2*rt->size;
        rt->times_ptr = tmp;
    }
    rt->count++;
    rt->times[rt->count] = value;
}
void response_times_append(ResponseTimesRef dest, ResponseTimesRef src)
{
    if(dest->count + src->count > dest->size) {
        int newsize = sizeof(double) * (dest->count + src->count);
        double* tmp = malloc(newsize);
        for(int i = 0; i < dest->size; i++)
            tmp[i] = dest->times[i];
        memcpy(tmp, dest->times_ptr, sizeof(double) * dest->size);
        dest->size = newsize;
    }
    for(int i = 0; i < src->count; i++) {
        dest->count++;
        dest->times[dest->count] = src->times[i];
    }
}
void append_thread_response_times(double all[MAX_RESPONSE_TIMES], const double thread_response_times[MAX_ROUNDTRIPS_PER_THREAD], int thread_ix)
{
    for(int i = 0; i < MAX_ROUNDTRIPS_PER_THREAD; i++) {
        double v = thread_response_times[i];
        all[(thread_ix * NBR_THREADS) + i] = thread_response_times[i];
    }
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
    total = 0.0;
    for(int i = 0; i < MAX_RESPONSE_TIMES; i++) {
        double v = all[i];
        total = total+(v-mean)*(v-mean);
    }
    double variance = total / (MAX_RESPONSE_TIMES * 1.0);
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
