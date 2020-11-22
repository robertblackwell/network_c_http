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
typedef struct DisarmTestCtx_s DisarmTestCtx;

struct DisarmTestCtx_s {
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    XrTimerWatcherRef   rearm_tw;
    DisarmTestCtx*      rearm_ctx;

};

DisarmTestCtx* DisarmTestCtx_new(int counter_init, int counter_max)
{
    DisarmTestCtx* tmp = malloc(sizeof(DisarmTestCtx));
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->start_time = current_time();
    return tmp;
}

static void callback_disarm_1(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    DisarmTestCtx* ctx_p = (DisarmTestCtx*) ctx;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(watcher->interval) - gap)/((double)(watcher->interval))));
    gap = percent_error;

    printf("inside timer callback_disarm_1 counter: %d %%error: %f   event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld\n", ctx_p->counter, gap, event, epollin, error);

    if(ctx_p->rearm_tw == NULL) {
        printf("callback_rearm_1 disarm timer \n");
        Xrtw_disarm(watcher);
        ctx_p->counter++;
    } else {
        if(ctx_p->counter >= ctx_p->max_count) {
            printf("rearmer if done \n");
            Xrtw_clear(ctx_p->rearm_tw);
            Xrtw_clear(watcher);
        } else {
            printf("rearming \n");
            Xrtw_rearm(ctx_p->rearm_tw, &callback_disarm_1, ctx_p->rearm_ctx, 1000, true);
            ctx_p->counter++;
        }
    }
}
int test_timer_disarm()
{

    DisarmTestCtx* test_ctx_p_1 = DisarmTestCtx_new(0, 1);
    XrRunloopRef rl = XrRunloop_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rl);

    Xrtw_set(tw_1, &callback_disarm_1, test_ctx_p_1, 1000, true);

    XrRunloop_run(rl, 10000);
    XrRunloop_free(rl);
    UT_EQUAL_INT(test_ctx_p_1->counter, 0);
    free(test_ctx_p_1);
    return 0;

}

int test_timer_disarm_rearm()
{

    DisarmTestCtx* test_ctx_p_1 = DisarmTestCtx_new(0, 7);
    DisarmTestCtx* test_ctx_p_2 = DisarmTestCtx_new(0, 6);

    XrRunloopRef rl = XrRunloop_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rl);
    XrTimerWatcherRef tw_2 = Xrtw_new(rl);
    // time 2 callback will rearm tw_1 after count of 5
    test_ctx_p_2->rearm_tw = tw_1;
    test_ctx_p_2->rearm_ctx = test_ctx_p_1;
    test_ctx_p_1->rearm_tw = NULL;
    test_ctx_p_1->rearm_ctx = NULL;



    Xrtw_set(tw_1, &callback_disarm_1, test_ctx_p_1, 1000, true);
    Xrtw_set(tw_2, &callback_disarm_1, test_ctx_p_2, 2000, true);

    XrRunloop_run(rl, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    XrRunloop_free(rl);
    return 0;
}



int main()
{
//    UT_ADD(test_timer_disarm);
    UT_ADD(test_timer_disarm_rearm);
    int rc = UT_RUN();
    return rc;
}
