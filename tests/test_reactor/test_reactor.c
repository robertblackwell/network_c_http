#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <math.h>
#include <c_http/list.h>
#include <c_http/operation.h>
#include <c_http/oprlist.h>
#include <c_http/unittest.h>
#include <c_http/utils.h>
#include <c_http/xr/runloop.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/twatcher.h>

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

typedef struct TestCtx_s {
    int                 counter;
    int                 max_count;

    struct timespec     start_time;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->start_time = current_time();
    return tmp;
}


//static int tw_counter_1 = 0;
static void callback_1(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    printf("inside timer callback_1 counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld\n", ctx_p->counter, gap, event, epollin, error);
    if(ctx_p->counter >= ctx_p->max_count) {
        printf("callback_1 clear timer \n");
        Xrtw_clear(watcher);
    } else {
        ctx_p->counter++;
    }
}

//static int tw_counter_2 = 0;
static void callback_2(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;
    ctx_p->start_time = tnow;

    printf("inside timer callback_2 counter: %d  %%error : %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld\n", ctx_p->counter, gap, event, epollin, error);
    if(ctx_p->counter >= 6) {
        printf("tw_callback_2 clear timer \n");
        Xrtw_clear(watcher);
    } else {
        ctx_p->counter++;
    }
}

//static int tw_counter_3 = 0;
static void callback_non_repeating(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    printf("inside timer callback_non_epeating counter : %d  %%error: %f event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld\n", ctx_p->counter, gap, event, epollin, error);
    ctx_p->counter++;
}

int test_timer_single_repeating()
{
    TestCtx* test_ctx_p = TestCtx_new(0, 5);

    XrRunloopRef rl = XrRunloop_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rl);

    Xrtw_set(tw_1, &callback_1, (void*)test_ctx_p, 1000, true);

    XrRunloop_run(rl, 10000);
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    XrRunloop_free(rl);
    return 0;
}

int test_timer_multiple_repeating()
{

    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    XrRunloopRef rl = XrRunloop_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rl);
    XrTimerWatcherRef tw_2 = Xrtw_new(rl);

    Xrtw_set(tw_1, &callback_1, test_ctx_p_1, 1000, true);
    Xrtw_set(tw_2, &callback_1, test_ctx_p_2, 1000, true);

    XrRunloop_run(rl, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    XrRunloop_free(rl);
    return 0;
}
int test_timer_non_repeating()
{

    TestCtx* test_ctx_p_1 = TestCtx_new(0, 1);
    XrRunloopRef rl = XrRunloop_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rl);

    Xrtw_set(tw_1, &callback_non_repeating, test_ctx_p_1, 1000, false);

    XrRunloop_run(rl, 10000);
    XrRunloop_free(rl);
    UT_EQUAL_INT(test_ctx_p_1->counter, 1);
    free(test_ctx_p_1);
    return 0;

}

int main()
{
    UT_ADD(test_timer_single_repeating);
    UT_ADD(test_timer_multiple_repeating);
    UT_ADD(test_timer_non_repeating);
    int rc = UT_RUN();
    return rc;
}
