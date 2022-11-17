#define _GNU_SOURCE
#define ENABLE_LOG
#include <c_http/async/types.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <c_http/logger.h>
#include <c_http/unittest.h>
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
//
// demonstrates that for a timer
// the sequence :
//      register
//          arm
//          disarm
//          arm
//          disarm
//      deregister
//
// works as expected
//
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
    bool                was_disarmed;
    bool                rearm_other;
    struct timespec     start_time;
    RtorTimerRef   other_tw;
    DisarmTestCtx*      other_ctx;
    long                interval_ms;

};

DisarmTestCtx* DisarmTestCtx_new(int counter_init, int counter_max)
{
    DisarmTestCtx* tmp = malloc(sizeof(DisarmTestCtx));
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->start_time = current_time();
    tmp->was_disarmed = false;
    return tmp;
}
// disarms itself on first call. If called subsequently and reaches its max then cancel both timers
static void callback_disarm_clear(RtorTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    DisarmTestCtx* ctx_p = (DisarmTestCtx*) ctx;

    // if first call disarm
    LOG_FMT(" counter %d\n", ctx_p->counter);
    if(ctx_p->counter <= 0) {
        LOG_MSG(" disarm self");
        rtor_timer_disarm(watcher);
        ctx_p->was_disarmed = true;
        ctx_p->counter++;
    } else {
        if(ctx_p->counter >= ctx_p->max_count) {
            LOG_MSG("disarm_cb clear other and self");
            rtor_timer_clear(ctx_p->other_tw);
            rtor_timer_clear(watcher);
            return;
        }
        ctx_p->counter++;
    }
}
// after being called max_count times rearm the other
static void callback_rearm_other(RtorTimerRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    DisarmTestCtx* ctx_p = (DisarmTestCtx*) ctx;

    int x = (ctx_p->counter >= ctx_p->max_count);
    int x2 = ((ctx_p->other_ctx->was_disarmed) && (ctx_p->counter >= ctx_p->max_count));
    LOG_FMT(" %d counter %d max %d \n", (int)ctx_p->other_ctx->was_disarmed, ctx_p->counter, ctx_p->max_count);
    LOG_FMT(" x %d x2 %d \n", x, x2);
    if((ctx_p->other_ctx->was_disarmed) && (ctx_p->counter == ctx_p->max_count)) {
        // other should be disarmed by now
        assert(ctx_p->other_ctx->was_disarmed);
        LOG_MSG("rearming other");
        rtor_timer_rearm(ctx_p->other_tw);
    }
    // keep counting until we are stopped by the other
    ctx_p->counter++;
    LOG_FMT(" counter %d\n", ctx_p->counter);
}
//
// timer 2 - disarms itself on the first event.
// timer 1 - after a number of repeat calls it rearms timer 2 .. timer 1 keeps repeating
// timer 2 - once it hits its max_count it clears or deregisters both timers allowing the reactor to return
//
int test_timer_disarm_rearm()
{

    DisarmTestCtx* test_ctx_p_1 = DisarmTestCtx_new(0, 7);
    test_ctx_p_1->interval_ms = 100;
    DisarmTestCtx* test_ctx_p_2 = DisarmTestCtx_new(0, 6);
    test_ctx_p_2->interval_ms = 100;

    ReactorRef rtor_ref = rtor_new();

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref, &callback_disarm_clear, test_ctx_p_1, test_ctx_p_1->interval_ms, true);
    RtorTimerRef tw_2 = rtor_timer_new(rtor_ref, &callback_rearm_other, test_ctx_p_2, test_ctx_p_2->interval_ms, true);
    // timer 1 callback will disarm itself on the first timer event
    // time 2 callback will rearm tw_1 after count of 5
    test_ctx_p_2->other_tw = tw_1;
    test_ctx_p_2->other_ctx = test_ctx_p_1;
    test_ctx_p_1->other_tw = tw_2;
    test_ctx_p_1->other_ctx = test_ctx_p_2;


    rtor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);

    //cannot predict how many times the rearm cb will be called before the disarm_clear cb final stops everything
    UT_TRUE(test_ctx_p_2->counter >= test_ctx_p_2->max_count);

    free(test_ctx_p_1);
    free(test_ctx_p_2);
    rtor_free(rtor_ref);
    return 0;
}



int main()
{
//    UT_ADD(test_timer_disarm);
    UT_ADD(test_timer_disarm_rearm);
    int rc = UT_RUN();
    return rc;
}
