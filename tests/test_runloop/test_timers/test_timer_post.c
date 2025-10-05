
#define ENABLE_LOGx
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

#include "./timer_helpers.c"
//
// post test -demonstrates that an event callback can post another handler which can post yet another handler
// and that second handler can cancel a fd based watcher.
//
static void posted_from_post_cb(RunloopRef runloop_ref, void* arg)
{
    TestCtx* ctx_p = arg;
    RunloopEventRef tw =  ctx_p->watcher; // (RunloopEventRef)w;
    RBL_LOG_FMT(" arg: %p  counter: %d", arg, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer ");
        runloop_timer_deregister(tw);
    } else {
        ctx_p->counter++;
    }
}
//static void post_cb(RunloopWatcherBaseRef w, void* arg, uint64_t event)
//{
//    TestCtx* ctx_p = arg;
//    RunloopEventRef tw = (RunloopEventRef)w;
//    RunloopRef reactor = w->runloop;
//    RBL_LOG_FMT(" post again w: %p counter: %d", w, ctx_p->counter);
//    runloop_post(reactor, posted_from_post_cb, ctx_p);
//}

static void callback_post(RunloopRef rl, void* ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*)ctx_arg;
    RunloopEventRef watcher = ctx_p->watcher;
    long interval = ctx_p->interval_ms;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(interval) - gap)/((double)(interval))));
    gap = percent_error;
    RunloopRef reactor = runloop_timer_get_runloop(watcher);
    RBL_LOG_FMT(" counter: %d gap: %f ", ctx_p->counter, gap);
    runloop_post(reactor, posted_from_post_cb, ctx_p);
}
int test_timer_post()
{
    // counter starts at 0 and increments to max 5
    RunloopRef runloop_ref = runloop_new();
    long interval_ms = 100;

    RunloopEventRef tw_1 = runloop_timer_new(runloop_ref);
    TestCtx* test_ctx_p = TestCtx_new(runloop_ref, tw_1, 0, 5, interval_ms);
    runloop_timer_register(tw_1, &callback_post, (void *) test_ctx_p, interval_ms, true);
    test_ctx_p->watcher = tw_1;

    runloop_run(runloop_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);
    return 0;
}


int main()
{
    UT_ADD(test_timer_post);
    int rc = UT_RUN();
    return rc;
}
