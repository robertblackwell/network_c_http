
#define RBL_LOG_ENABLE 1
#define RBL_LOG_ALLOW_GLOBAL 1
#include <http_in_c/async-old/types.h>
#include <stdio.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <math.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
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
    int                 selector; // selects which test to run in each thread
    pthread_t           thread;
    int                 counter;    //  How many times a callback received this instance as a void* argument
    int                 max_count;  //  The expected maximum tnumber of times a callback should be called with this
                                    //  instance
    struct timespec     start_time; //  the time at which the timer was first scheduled
    RunloopRef          reactor;    //  the reactor being used for the timer
    RunloopTimerRef     watcher;    //  the timer being used for this experiment
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//////////////////////////////////////////////////////////////////////////////////////////////////////
// A callback for the non repeating test.
// It expects to get called with the ctx argument as a void* pointing to an instance of TestCtx.
// The ctx arg contains enough info for the callback to calculate the margin by which it failed to
// be called when it was expected to be called
///////////////////////////////////////////////////////////////////////////////////////////////////////
static void callback_non_repeating(RunloopTimerRef watcher, RunloopTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    RBL_LOG_FMT("counter : %d  %%error: %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    ctx_p->counter++;
}
/**
 * A test harness for demonstrating a non-repeating timer.
 * A timer is initiated and the callback increments a counter on each call.
 * After a set number of calls to the callback
 * The timer is cancelled by the callback
 * Cancellation of the timer causes the runloop to end
 * After end of the runloop the TestCtx->counter is verified to be equal to the
 * required number of invocations
 */
int test_timer_non_repeating()
{

    TestCtx* test_ctx_p_1 = TestCtx_new(0, 1);
    // previous call sets test_ctx_p_1->counter == 0

    RunloopRef runloop_ref = runloop_new();

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_non_repeating, test_ctx_p_1, 100, false);

    runloop_run(runloop_ref, 10000);
    /*We should only get here when there are no more timers or other events pending in the runloop*/
    runloop_free(runloop_ref);
    /* prove callback_non_repeating was called exactly once */
    UT_EQUAL_INT(test_ctx_p_1->counter, 1);
    free(test_ctx_p_1);
    return 0;
}
//
/* single repeating timer
* A timer is initiated and the callback increments a counter on each call.
* After a set number of calls to the callback
* The timer is cancelled by the callback
* Cancellation of the timer causes the runloop to end
* After end of the runloop the TestCtx->counter is verified to be equal to the
* required number of invocations
*/
static void callback_repeating(RunloopTimerRef watcher, RunloopTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    RBL_LOG_FMT("counter: %d %%error: %f   ", ctx_p->counter, gap);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer");
        runloop_timer_deregister(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_single_repeating()
{
    // counter starts at 0 and increments to max 5
    TestCtx* test_ctx_p = TestCtx_new(0, 5);

    RunloopRef runloop_ref = runloop_new();

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_repeating, (void *) test_ctx_p, 100, true);

    runloop_run(runloop_ref, 10000);
    RBL_LOG_MSG("timer_single_repeating - runloop_run has exited")
    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);

    return 0;
}
//
// multiple repeating times
//
//static int tw_counter_2 = 0;
static void callback_multiple_repeating_timers(RunloopTimerRef watcher, RunloopTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    ctx_p->start_time = tnow;

    RBL_LOG_FMT(" counter: %d  ctx: %p  event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, ctx_p, event, epollin, error);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_FMT(" clear timer %p", ctx_p);
        runloop_timer_deregister(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_multiple_repeating()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    RunloopRef runloop_ref = runloop_new();

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_multiple_repeating_timers, test_ctx_p_1, 100, true);

    RunloopTimerRef tw_2 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_2, &callback_multiple_repeating_timers, test_ctx_p_2, 100, true);

    runloop_run(runloop_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    runloop_free(runloop_ref);
    return 0;
}
//
// post test -demonstrates that an event callback can post another handler which can post yet another handler
// and that second handler can cancel a fd based watcher.
//
static void posted_from_post_cb(RunloopRef runloop_ref, void* arg)
{
    TestCtx* ctx_p = arg;
    RunloopTimerRef tw =  ctx_p->watcher; // (RunloopTimerRef)w;
    RBL_LOG_FMT(" arg: %p  counter: %d", arg, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer ");
        runloop_timer_deregister(tw);
    } else {
        ctx_p->counter++;
    }
}
//static void post_cb(RunloopWatcherRef w, void* arg, uint64_t event)
//{
//    TestCtx* ctx_p = arg;
//    RunloopTimerRef tw = (RunloopTimerRef)w;
//    RunloopRef reactor = w->runloop;
//    RBL_LOG_FMT(" post again w: %p counter: %d", w, ctx_p->counter);
//    runloop_post(reactor, posted_from_post_cb, ctx_p);
//}

static void callback_post(RunloopTimerRef watcher, RunloopTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    RunloopRef reactor = watcher->runloop;
    RBL_LOG_FMT(" counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    runloop_post(reactor, posted_from_post_cb, ctx_p);
}
int test_timer_post()
{
    // counter starts at 0 and increments to max 5
    RunloopRef runloop_ref = runloop_new();
    TestCtx* test_ctx_p = TestCtx_new(0, 5);
    test_ctx_p->reactor = runloop_ref;

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_post, (void *) test_ctx_p, 100, true);
    test_ctx_p->watcher = tw_1;

    runloop_run(runloop_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);
    return 0;
}


static struct timespec current_time()
{
    struct timespec ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}
static double time_diff(struct timespec a, struct timespec b)
{
    struct timespec result;
    result.tv_nsec = a.tv_nsec - b.tv_nsec;
    result.tv_sec = a.tv_sec - b.tv_sec;

    double rdouble = (double)(result.tv_sec)*(1000.0) + ((double)(result.tv_nsec)/1000.0);
    return rdouble;
}

TestCtx* TestCtx_new(int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->start_time = current_time();
    tmp->selector = 1;
    return tmp;
}
void testCtx_dispose(TestCtx* this)
{
    free(this);
}
void* threadfn(void* arg)
{
    TestCtx* ctx = (TestCtx*)arg;
    if(ctx->selector == 1)
        test_timer_single_repeating();
    else
        test_timer_multiple_repeating();
    return NULL;
}
int make_multiple_threads(int selector)
{
    printf("test_timer_multithread\n");
    int nbr_threads = 5;
    TestCtx* contexts[nbr_threads];
    for(int ix = 0; ix < 5; ix++) {
        contexts[ix] = TestCtx_new(0, 10);
        contexts[ix]->selector = selector;
        int r = pthread_create(&(contexts[ix]->thread), NULL, &threadfn, (void*)&(contexts[ix]));

    }
    for(int ix = 0; ix < 5; ix++) {
        pthread_join(contexts[ix]->thread, NULL);
    }
    for(int ix = 0; ix < 5; ix++) {
        free(contexts[ix]);
    }
    return 0;
}
int test_multiple_threads_single_repeaters() {
    make_multiple_threads(1);
}
int test_multiple_threads_multiple_repeaters() {
    make_multiple_threads(2);
}
int main()
{
    printf("Testing multithreading wth runloop\n");
    UT_ADD(test_multiple_threads_single_repeaters);
    UT_ADD(test_multiple_threads_multiple_repeaters);
    int rc = UT_RUN();
    return rc;
}
