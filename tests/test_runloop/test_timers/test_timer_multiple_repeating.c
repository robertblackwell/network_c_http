
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
#include "./timer_multiple_repeating.c"
#if 0
//
// multiple repeating times
//
//static int tw_counter_2 = 0;
static void callback_multiple_repeating_timers(RunloopRef runloop_ref, void* ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*) ctx_arg;
    long interval = ctx_p->interval_ms;
    RunloopTimerRef watcher = ctx_p->watcher;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    double percent_error = fabs(100.0*(((double)(interval) - gap)/((double)(interval))));
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
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5, 100);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6, 100);

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
#endif
int main()
{
    UT_ADD(test_timer_multiple_repeating);
    int rc = UT_RUN();
    return rc;
}
