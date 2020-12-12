#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include <c_http/xr/types.h>
#include <stdio.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <math.h>
#include <c_http/unittest.h>
#include <c_http/utils.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/w_timer.h>
//
// A)
// theses tests deomstrate that a timer is called the expected number of times,
// that the timer callback can cancel the timer
// when all timers have been cancelled the reactor::run() function exits
//
// b) how to "post" a callback function to run on the "next tick" in javascript language
//
static struct timespec current_time();
static double time_diff(struct timespec a, struct timespec b);

typedef struct TestCtx_s  {
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    XrReactorRef        reactor;
    WTimerRef   watcher;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//
// non repeating test
//
static void callback_non_repeating(WTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    XR_TRACE("counter : %d  %%error: %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    ctx_p->counter++;
}

int test_timer_non_repeating()
{

    TestCtx* test_ctx_p_1 = TestCtx_new(0, 1);
    // previous call sets test_ctx_p_1->counter == 0

    XrReactorRef rtor_ref = XrReactor_new();

    WTimerRef tw_1 = WTimer_new(rtor_ref, &callback_non_repeating, test_ctx_p_1, 100, false);

    XrReactor_run(rtor_ref, 10000);
    XrReactor_free(rtor_ref);
    // prove callback_non_repeating was called exactly once
    UT_EQUAL_INT(test_ctx_p_1->counter, 1);
    free(test_ctx_p_1);
    return 0;
}
//
// single repeating timer
//

//static int tw_counter_1 = 0;
static void callback_1(WTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    XR_TRACE("counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    if(ctx_p->counter >= ctx_p->max_count) {
        XR_TRACE_MSG(" clear timer");
        WTimer_clear(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_single_repeating()
{
    // counter starts at 0 and increments to max 5
    TestCtx* test_ctx_p = TestCtx_new(0, 5);

    XrReactorRef rtor_ref = XrReactor_new();

    WTimerRef tw_1 = WTimer_new(rtor_ref, &callback_1, (void*)test_ctx_p, 100, true);

    XrReactor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    XrReactor_free(rtor_ref);
    return 0;
}
//
// multiple repeating times
//
//static int tw_counter_2 = 0;
static void callback_2(WTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    ctx_p->start_time = tnow;

    XR_TRACE(" counter: %d  %%error : %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    if(ctx_p->counter >= 6) {
        XR_TRACE_MSG(" clear timer ");
        WTimer_clear(watcher);
    } else {
        ctx_p->counter++;
    }
}
int test_timer_multiple_repeating()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    XrReactorRef rtor_ref = XrReactor_new();

    WTimerRef tw_1 = WTimer_new(rtor_ref, &callback_1, test_ctx_p_1, 100, true);
    WTimerRef tw_2 = WTimer_new(rtor_ref, &callback_1, test_ctx_p_2, 100, true);

    XrReactor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    XrReactor_free(rtor_ref);
    return 0;
}
//
// post test -demonstrates that an event callback can post another handler which can post yet another handler
// and that second handler can cancel a fd based watcher.
//
static void posted_from_post_cb(void* arg)
{
    TestCtx* ctx_p = arg;
    WTimerRef tw =  ctx_p->watcher; // (WTimerRef)w;
    XR_TRACE(" arg: %p  counter: %d", arg, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        XR_TRACE_MSG(" clear timer ");
        WTimer_clear(tw);
    } else {
        ctx_p->counter++;
    }
}
static void post_cb(WatcherRef w, void* arg, uint64_t event)
{
    TestCtx* ctx_p = arg;
    WTimerRef tw = (WTimerRef)w;
    XrReactorRef reactor = w->runloop;
    XR_TRACE(" post again w: %p counter: %d", w, ctx_p->counter);
    XrReactor_post(reactor, posted_from_post_cb, ctx_p);
}

static void callback_post(WTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    XrReactorRef reactor = watcher->runloop;
    XR_TRACE(" counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", ctx_p->counter, gap, event, epollin, error);
    XrReactor_post(reactor, posted_from_post_cb, ctx);
}
int test_timer_post()
{
    // counter starts at 0 and increments to max 5
    XrReactorRef rtor_ref = XrReactor_new();
    TestCtx* test_ctx_p = TestCtx_new(0, 5);
    test_ctx_p->reactor = rtor_ref;

    WTimerRef tw_1 = WTimer_new(rtor_ref, &callback_post, (void*)test_ctx_p, 100, true);
    test_ctx_p->watcher = tw_1;

    XrReactor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    XrReactor_free(rtor_ref);
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
