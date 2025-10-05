
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

//////////////////////////////////////////////////////////////////////////////////////////////////////
// A callback for the non repeating test.
// It expects to get called with the ctx argument as a void* pointing to an instance of TestCtx.
// The ctx arg contains enough info for the callback to calculate the margin by which it failed to
// be called when it was expected to be called
///////////////////////////////////////////////////////////////////////////////////////////////////////
static void callback_non_repeating(RunloopRef runloop, void* test_ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*) test_ctx_arg;
    long interval = ctx_p->interval_ms;
    struct timespec tnow = current_time();
    double gap = time_diff(tnow, ctx_p->start_time);
    ctx_p->start_time = tnow;
    double percent_error = fabs(100.0*(((double)(interval) - gap)/((double)(interval))));
    gap = percent_error;
    ctx_p->counter++;
    RunloopTimerRef tr = ctx_p->watcher;
    runloop_timer_free(tr);
    RBL_LOG_FMT("counter : %d  gap: %f ", ctx_p->counter, gap);
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

    // previous call sets test_ctx_p_1->counter == 0

    RunloopRef runloop_ref = runloop_new();
    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);

    TestCtx* test_ctx_p_1 = TestCtx_new(runloop_ref, tw_1, 1, 5, 100);

    runloop_timer_register(tw_1, &callback_non_repeating, test_ctx_p_1, 5000, false);
    runloop_run(runloop_ref, 100000);
    /*We should only get here when there are no more timers or other events pending in the runloop*/
    runloop_free(runloop_ref);
    /* prove callback_non_repeating was called exactly once */
    UT_EQUAL_INT(test_ctx_p_1->counter, 2);
    free(test_ctx_p_1);
    return 0;
}

int main()
{
    UT_ADD(test_timer_non_repeating);
    int rc = UT_RUN();
    return rc;
}
