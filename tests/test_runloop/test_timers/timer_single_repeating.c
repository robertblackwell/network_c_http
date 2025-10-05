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
/* single repeating timer
* A timer is initiated and the callback increments a counter on each call.
* After a set number of calls to the callback
* The timer is cancelled by the callback
* Cancellation of the timer causes the runloop to end
* After end of the runloop the TestCtx->counter is verified to be equal to the
* required number of invocations
*/
static void callback_repeating(RunloopRef runloop_ref, void* test_ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*) test_ctx_arg;
    long interval = ctx_p->interval_ms;
    RunloopTimerRef watcher = ctx_p->watcher;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(interval) - gap)/((double)(interval))));
    gap = percent_error;

    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_FMT(" clear timer watcher: %p", watcher);
        runloop_timer_deregister(watcher);
    } else {
        RBL_LOG_FMT("callback_repeating counter: %d", ctx_p->counter);
        ctx_p->counter++;
    }
}
int test_timer_single_repeating()
{
    // counter starts at 0 and increments to max 5
    long interval_ms = 100;
    RunloopRef runloop_ref = runloop_new();

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    TestCtx* test_ctx_p = TestCtx_new(runloop_ref, tw_1, 0, 5, interval_ms);
    runloop_timer_register(tw_1, &callback_repeating, (void *) test_ctx_p, 100, true);
    runloop_run(runloop_ref, 10000);
    RBL_LOG_MSG("timer_single_repeating - runloop_run has exited")
    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);

    return 0;
}
