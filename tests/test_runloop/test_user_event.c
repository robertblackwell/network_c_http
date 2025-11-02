#include <stdio.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <pthread.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>

#define TestCtx_TYPE "Tcxtx"
typedef struct TestCtx_s  {
    RBL_DECLARE_TAG;
    int                 ident;
    int                 callback1_counter;
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    RunloopRef          runloop_ref;
    RunloopTimerRef     timer_ref;
    RunloopUserEventRef user_event_ref;
    int                 fdevent_counter;
    RBL_DECLARE_END_TAG;
} TestCtx;

TestCtx* TestCtx_new(RunloopRef rl, int id, RunloopTimerRef timer_ref, RunloopUserEventRef eventfd_ref, int counter_init, int counter_max);
TestCtx* TestCtx_new(RunloopRef rl, int id, RunloopTimerRef timer_ref, RunloopUserEventRef eventfd_ref,int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    RBL_SET_TAG(TestCtx_TYPE, tmp)
    RBL_SET_END_TAG(TestCtx_TYPE, tmp)
    tmp->ident = id;
    tmp->counter = counter_init;
    tmp->callback1_counter = 0;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    tmp->runloop_ref = rl;
    tmp->timer_ref = timer_ref;
    tmp->user_event_ref = eventfd_ref;
    return tmp;
}

int test_userevent_multiple();
int test_userevent_1();
static void timer_callback_1(RunloopRef rl, void* arg);
void fdevent_postable(RunloopRef rl, void* test_ctx_arg);

int nbr_timers = 0;
int nbr_expected_triggers = 0;
int main()
{
    UT_ADD(test_userevent_1);
    UT_ADD(test_userevent_multiple);
    int rc = UT_RUN();
    return rc;
}
/**
 * Use a time to repeatedly fire a fdevent and check that the
 * fdevent watcher catches all the fired events.
 * All happens in a single thread
 */
#define NBR_TIMES_FIRE 10
int test_userevent_1()
{
    nbr_expected_triggers = 1;
    RunloopRef runloop_ref = runloop_new();
    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    RunloopUserEventRef fdev = runloop_user_event_new(runloop_ref);

    TestCtx* test_ctx_p = TestCtx_new(runloop_ref, 1, tw_1, fdev, 0, 10);

    runloop_timer_register(tw_1, &timer_callback_1, (void *) test_ctx_p, 100, true);
    runloop_timer_disarm(tw_1);
    nbr_timers = 1;
    runloop_user_event_register(fdev);
    runloop_user_event_arm(fdev, &fdevent_postable, test_ctx_p);
    runloop_timer_rearm(tw_1);
    runloop_run(runloop_ref, 5000);

    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);
    return 0;
}

int test_userevent_multiple()
{
    nbr_expected_triggers = 11;
    RunloopRef runloop_ref = runloop_new();
    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    RunloopUserEventRef fdev = runloop_user_event_new(runloop_ref);

    TestCtx* test_ctx_p_1 = TestCtx_new(runloop_ref, 1, tw_1, fdev, 0, 5);
    RunloopTimerRef tw_2 = runloop_timer_new(runloop_ref);
    TestCtx* test_ctx_p_2 = TestCtx_new(runloop_ref, 2, tw_2, fdev, 0, 6);
    nbr_timers = 2;
    runloop_user_event_register(fdev);
    runloop_user_event_arm(fdev, &fdevent_postable, test_ctx_p_1);

    runloop_timer_register(tw_1, &timer_callback_1, test_ctx_p_1, 100, true);
    runloop_timer_register(tw_2, &timer_callback_1, test_ctx_p_2, 100, true);

    runloop_run(runloop_ref, 5000);
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
    RunloopUserEventRef fdevent_ref = ctx_p->user_event_ref;
    RBL_ASSERT((fdevent_ref != NULL), "callback1 fdevent_ref == NULL");
    RBL_LOG_FMT("callback1_counter %d counter: %d", ctx_p->callback1_counter, ctx_p->counter);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer");
        // runloop_close(ctx_p->runloop_ref);
        runloop_timer_deregister(ctx_p->timer_ref);
        runloop_timer_free(ctx_p->timer_ref);
        ctx_p->timer_ref = NULL;
        nbr_timers--;
    } else {
        runloop_user_event_fire(fdevent_ref);
        ctx_p->counter++;
    }
    ctx_p->callback1_counter++;
}
void fdevent_postable(RunloopRef rl, void* test_ctx_arg)
{
    TestCtx* ctx_p = (TestCtx*)test_ctx_arg;
    ctx_p->fdevent_counter++;
    printf("fdevent_postable fdevent_count: %d nbr_expected_triggers: %d \n", ctx_p->fdevent_counter, nbr_expected_triggers);
    RBL_LOG_FMT("w: %p arg: %p fdevent_counter % d", ctx_p->user_event_ref , ctx_p, ctx_p->fdevent_counter);
}
