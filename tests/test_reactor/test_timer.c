#define _GNU_SOURCE
#define ENABLE_LOGx
#include <c_http/async/types.h>
#include <stdio.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <math.h>
#include <c_http/logger.h>
#include <c_http/unittest.h>
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
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
    struct timespec     start_time; //  the time at which the timer was first scheduled
    ReactorRef          reactor;    //  the reactor being used for the timer
    RtorTimerRef         watcher;    //  the timer being used for this experiment
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//////////////////////////////////////////////////////////////////////////////////////////////////////
// A callback for the non repeating test.
// It expects to get called with the ctx argument as a void* pointing to an instance of TestCtx.
// The ctx arg contains enough info for the callback to calculate the margin by which it failed to
// be called when it was expected to be called
///////////////////////////////////////////////////////////////////////////////////////////////////////
static void callback_non_repeating(RtorTimerRef watcher, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    LOG_FMT("counter : %d  %%error: %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    ctx_p->counter++;
}
/**
 * A test harness for demonstrating a non-repeating timer.
 * A timer is initiated and the callback increments a counter on each call.
 * After a set number of calls to the callback
 * The timer is cancelled by the callback
 * Cancellation of the timer causes the simple_runloop to end
 * After end of the simple_runloop the TestCtx->counter is verified to be equal to the
 * required number of invocations
 */
int test_timer_non_repeating()
{

    TestCtx* test_ctx_p_1 = TestCtx_new(0, 1);
    // previous call sets test_ctx_p_1->counter == 0

    ReactorRef rtor_ref = rtor_new();

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_non_repeating, test_ctx_p_1, 100, false);

    rtor_run(rtor_ref, 10000);
    /*We should only get here when there are no more timers or other events pending in the simple_runloop*/
    rtor_free(rtor_ref);
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
* Cancellation of the timer causes the simple_runloop to end
* After end of the simple_runloop the TestCtx->counter is verified to be equal to the
* required number of invocations
*/
static void callback_repeating(RtorTimerRef watcher, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    LOG_FMT("counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    if(ctx_p->counter >= ctx_p->max_count) {
        LOG_MSG(" clear timer");
        rtor_timer_deregister(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_single_repeating()
{
    // counter starts at 0 and increments to max 5
    TestCtx* test_ctx_p = TestCtx_new(0, 5);

    ReactorRef rtor_ref = rtor_new();

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_repeating, (void *) test_ctx_p, 100, true);

    rtor_run(rtor_ref, 10000);
    LOG_MSG("timer_single_repeating - rtor_run has exited")
    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    rtor_free(rtor_ref);

    return 0;
}
//
// multiple repeating times
//
//static int tw_counter_2 = 0;
static void callback_multiple_repeating_timers(RtorTimerRef watcher, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    ctx_p->start_time = tnow;

    LOG_FMT(" counter: %d  ctx: %p  event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, ctx_p, event, epollin, error);
    if(ctx_p->counter >= ctx_p->max_count) {
        LOG_FMT(" clear timer %p", ctx_p);
        rtor_timer_deregister(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_multiple_repeating()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    ReactorRef rtor_ref = rtor_new();

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_multiple_repeating_timers, test_ctx_p_1, 100, true);

    RtorTimerRef tw_2 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_2, &callback_multiple_repeating_timers, test_ctx_p_2, 100, true);

    rtor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    rtor_free(rtor_ref);
    return 0;
}
//
// post test -demonstrates that an event callback can post another handler which can post yet another handler
// and that second handler can cancel a fd based watcher.
//
static void posted_from_post_cb(ReactorRef rtor_ref,  void* arg)
{
    TestCtx* ctx_p = arg;
    RtorTimerRef tw =  ctx_p->watcher; // (RtorTimerRef)w;
    LOG_FMT(" arg: %p  counter: %d", arg, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        LOG_MSG(" clear timer ");
        rtor_timer_deregister(tw);
    } else {
        ctx_p->counter++;
    }
}
//static void post_cb(RtorWatcherRef w, void* arg, uint64_t event)
//{
//    TestCtx* ctx_p = arg;
//    RtorTimerRef tw = (RtorTimerRef)w;
//    ReactorRef reactor = w->runloop;
//    LOG_FMT(" post again w: %p counter: %d", w, ctx_p->counter);
//    rtor_post(reactor, posted_from_post_cb, ctx_p);
//}

static void callback_post(RtorTimerRef watcher, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) watcher->timer_handler_arg;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    ReactorRef reactor = watcher->runloop;
    LOG_FMT(" counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    rtor_post(reactor, posted_from_post_cb, ctx_p);
}
int test_timer_post()
{
    // counter starts at 0 and increments to max 5
    ReactorRef rtor_ref = rtor_new();
    TestCtx* test_ctx_p = TestCtx_new(0, 5);
    test_ctx_p->reactor = rtor_ref;

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_post, (void *) test_ctx_p, 100, true);
    test_ctx_p->watcher = tw_1;

    rtor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    rtor_free(rtor_ref);
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
    return tmp;
}

int main()
{
    UT_ADD(test_timer_single_repeating);
    UT_ADD(test_timer_multiple_repeating);
    UT_ADD(test_timer_non_repeating);
    UT_ADD(test_timer_post);
    int rc = UT_RUN();
    return rc;
}
