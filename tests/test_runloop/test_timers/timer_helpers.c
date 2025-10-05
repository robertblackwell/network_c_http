#ifndef TIMER_HELPERS
#define TIMER_HELPERS
#include <stdio.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <math.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
// #include <src/runloop/rl_internal.h>
//
// A) These tests deomstrate that
// -    a timer is called the expected number of times,
// -    that the timer callback can cancel the timer
// -    when all timers have been cancelled the reactor::run() function exits
//
// B) how to "post" a callback function to run on the "next tick" in javascript language style
//
static struct timespec current_time();
static double time_diff(struct timespec a, struct timespec b);

typedef struct TestCtx_s  {
    int                 counter;    //  How many times a callback received this instance as a void* argument
    int                 max_count;  //  The expected maximum tnumber of times a callback should be called with this
                                    //  instance
    long                interval_ms;
    int selector;
    pthread_t thread;
    struct timespec     start_time;  //  the time at which the timer was first scheduled
    RunloopRef          runloop_ref; //  the reactor being used for the timer
    RunloopEventRef     watcher;     //  the timer being used for this experiment
} TestCtx;


static struct timespec current_time()
{
    struct timespec ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}
double time_diff(struct timespec a, struct timespec b)
{
    struct timespec result;
    result.tv_nsec = a.tv_nsec - b.tv_nsec;
    result.tv_sec = a.tv_sec - b.tv_sec;

    double rdouble = (double)(result.tv_sec)*(1000.0) + ((double)(result.tv_nsec)/1000.0);
    return rdouble;
}

TestCtx* TestCtx_new(RunloopRef rl, RunloopEventRef timer_ref,  int counter_init, int counter_max, long interval_ms)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    tmp->interval_ms = interval_ms;
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->start_time = current_time();
    tmp->selector = 1;
    tmp->runloop_ref = rl;
    tmp->watcher = timer_ref;
    return tmp;
}
#endif