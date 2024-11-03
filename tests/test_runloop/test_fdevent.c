
#define ENABLE_LOGx
#include <http_in_c/async-old/types.h>
#include <stdio.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <rbl/unittest.h>
#include <rbl/check_tag.h>
#include <rbl/macros.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
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
    RunloopRef          reactor;
    RunloopTimerRef         timer;
    RunloopEventfdRef         fdevent;
    int                 fdevent_counter;
    RBL_DECLARE_END_TAG;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//
// single repeating timer
//

int test_fdevent_multiple();
int test_fdevent_1();
static void callback_1(RunloopTimerRef watcher, RunloopTimerEvent event);
void fdevent_handler(RunloopEventfdRef fdev_ref, uint64_t ev_mask);


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
    TestCtx* test_ctx_p = TestCtx_new(0, NBR_TIMES_FIRE);

    RunloopRef runloop_ref = runloop_new();
    test_ctx_p->reactor = runloop_ref;
    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_1, (void *) test_ctx_p, 1000, true);
    runloop_timer_disarm(tw_1);
    RunloopEventfdRef fdev = runloop_eventfd_new(runloop_ref);

    test_ctx_p->fdevent = fdev;
    test_ctx_p->timer = tw_1;


    runloop_eventfd_register(fdev);
    runloop_eventfd_arm(fdev, &fdevent_handler, test_ctx_p);
    runloop_timer_rearm(tw_1);
    runloop_run(runloop_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    runloop_free(runloop_ref);
    return 0;
}

int test_fdevent_multiple()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    RunloopRef runloop_ref = runloop_new();

    RunloopTimerRef tw_1 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_1, &callback_1, test_ctx_p_1, 100, true);

    RunloopTimerRef tw_2 = runloop_timer_new(runloop_ref);
    runloop_timer_register(tw_2, &callback_1, test_ctx_p_2, 100, true);

    runloop_run(runloop_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    runloop_free(runloop_ref);
    return 0;
}
static void callback_1(RunloopTimerRef watcher, RunloopTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    void* ctx = watcher->timer_handler_arg;
    TestCtx* ctx_p = (TestCtx*) ctx;
    RunloopEventfdRef fdevent_ref = ctx_p->fdevent;
    RBL_ASSERT((fdevent_ref != NULL), "callback1 fdevent_ref == NULL");
    RBL_LOG_FMT("callback1_counter %d counter: %d event is : %lx  ", ctx_p->callback1_counter, ctx_p->counter, event);
    if(ctx_p->counter >= ctx_p->max_count) {
        RBL_LOG_MSG(" clear timer");
        runloop_close(ctx_p->reactor);
//        runloop_timer_deregister(watcher);
//        runloop_eventfd_deregister(fdev);
    } else {
        runloop_eventfd_fire(fdevent_ref);
        ctx_p->counter++;
        runloop_eventfd_fire(fdevent_ref);
        ctx_p->counter++;

    }
    ctx_p->callback1_counter++;
}
void fdevent_handler(RunloopEventfdRef fdev_ref, uint64_t ev_mask)
{
    void* arg = fdev_ref->fd_event_handler_arg;
    TestCtx* t = (TestCtx*)arg;
    t->fdevent_counter++;
    RBL_LOG_FMT("w: %p arg: %p ev mask: %ld fdevent_counter % d", fdev_ref , arg, ev_mask, t->fdevent_counter);
}

TestCtx* TestCtx_new(int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    RBL_SET_TAG(TestCtx_TYPE, tmp)
    RBL_SET_END_TAG(TestCtx_TYPE, tmp)
    tmp->counter = counter_init;
    tmp->callback1_counter = 0;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    return tmp;
}
