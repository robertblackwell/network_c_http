
#include <stdio.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include <src/runloop/kqueue_runloop/runloop.h>
#include <src/runloop/kqueue_runloop/rl_internal.h>
//
// Tests fdevent
// start two threads
//  thread A waits on a fdevent object
//  thread B is a repeating timer, each timer tick it fires the fdevent object and after 5 ticks it terminates both event sources
//
#define TestCtx_TYPE "Tcxtx"
typedef struct TestCtx_s  {
    RBL_DECLARE_TAG;
    int                 callback1_counter;
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    RunloopRef          runloop_ref;
    RunloopEventRef     timer_ref;
    RunloopEventRef     user_event_ref;
    int                 fdevent_counter;
    RBL_DECLARE_END_TAG;
} TestCtx;

TestCtx* TestCtx_new(RunloopRef rl, RunloopEventRef timer_ref, RunloopEventRef eventfd_ref, int counter_init, int counter_max);

//
// single repeating timer
//

int test_fdevent_multiple();
int test_fdevent_1();
static void timer_callback_1(RunloopRef rl, void* arg);
void fdevent_postable(RunloopRef rl, void* test_ctx_arg);


int main()
{
    UT_ADD(test_fdevent_1);
//    UT_ADD(test_fdevent_multiple);
    int rc = UT_RUN();
    return rc;
}
/**
 * Use a time to repeatedly fire a fdevent and check that the
 * fdevent watcher catches all the fired events.
 * All happens in a single thread
 */
#define NBR_TIMES_FIRE 10
int test_fdevent_1()
{

    RunloopRef runloop_ref = runloop_new();
    RunloopEventRef tw_1 = runloop_timer_new(runloop_ref);
    RunloopEventRef fdev = runloop_user_event_new(runloop_ref);

    TestCtx* test_ctx_p = TestCtx_new(runloop_ref, tw_1, fdev, 0, NBR_TIMES_FIRE);

    runloop_timer_register(tw_1, &timer_callback_1, (void *) test_ctx_p, 100, true);
    runloop_timer_disarm(tw_1);
    runloop_user_event_register(fdev);
    runloop_user_event_arm(fdev, &fdevent_postable, test_ctx_p);
    runloop_timer_rearm(tw_1);
    runloop_run(runloop_ref, 10000);

    /*
     * Test that callback_1 was called as many times as the
     * assert counter was increment correct number of times
    */
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
//    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);
    return 0;
}

int test_fdevent_multiple()
{

    RunloopRef runloop_ref = runloop_new();
    RunloopEventRef tw_1 = runloop_timer_new(runloop_ref);
    RunloopEventRef fdev = runloop_user_event_new(runloop_ref);

    TestCtx* test_ctx_p_1 = TestCtx_new(runloop_ref, tw_1, fdev, 0, 5);
    RunloopEventRef tw_2 = runloop_timer_new(runloop_ref);
    TestCtx* test_ctx_p_2 = TestCtx_new(runloop_ref, tw_2, fdev, 0, 6);

    runloop_user_event_register(fdev);
    runloop_user_event_arm(fdev, &fdevent_postable, test_ctx_p_1);

//    runloop_timer_register(tw_1, &callback_1, (void *) test_ctx_p_1, 1000, true);
//    runloop_timer_disarm(tw_1);
//    runloop_timer_rearm(tw_1);
//
//    runloop_timer_register(tw_2, &callback_1, (void *) test_ctx_p_2, 1000, true);
//    runloop_timer_disarm(tw_1);
//    runloop_timer_rearm(tw_1);


    runloop_timer_register(tw_1, &timer_callback_1, test_ctx_p_1, 100, true);
    runloop_timer_register(tw_2, &timer_callback_1, test_ctx_p_2, 100, true);

    runloop_run(runloop_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    runloop_free(runloop_ref);
    return 0;
}
static void timer_callback_1(RunloopRef rl, void* test_ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*) test_ctx_arg;
    RunloopEventRef fdevent_ref = ctx_p->user_event_ref;
    RBL_ASSERT((fdevent_ref != NULL), "callback1 fdevent_ref == NULL");
    RBL_LOG_FMT("callback1_counter %d counter: %d", ctx_p->callback1_counter, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer");
        runloop_close(ctx_p->runloop_ref);
//        runloop_timer_deregister(watcher);
//        runloop_eventfd_deregister(fdev);
    } else {
        runloop_user_event_fire(fdevent_ref);
        ctx_p->counter++;
        runloop_user_event_fire(fdevent_ref);
        ctx_p->counter++;

    }
    ctx_p->callback1_counter++;
}
void fdevent_postable(RunloopRef rl, void* test_ctx_arg)
{
    TestCtx* test_ctx_p = (TestCtx*)test_ctx_arg;
    test_ctx_p->fdevent_counter++;
    RBL_LOG_FMT("w: %p arg: %p fdevent_counter % d", test_ctx_p->user_event_ref , test_ctx_arg, test_ctx_p->fdevent_counter);
}

TestCtx* TestCtx_new(RunloopRef rl, RunloopEventRef timer_ref, RunloopEventRef user_event_ref,int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    RBL_SET_TAG(TestCtx_TYPE, tmp)
    RBL_SET_END_TAG(TestCtx_TYPE, tmp)
    tmp->counter = counter_init;
    tmp->callback1_counter = 0;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    tmp->runloop_ref = rl;
    tmp->timer_ref = timer_ref;
    tmp->user_event_ref = user_event_ref;
    return tmp;
}
