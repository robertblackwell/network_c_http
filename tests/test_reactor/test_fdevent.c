#define _GNU_SOURCE
#define ENABLE_LOG
#include <c_http/async/types.h>
#include <stdio.h>
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
// Tests fdevent
// start two threads
//  thread A waits on a fdevent object
//  thread B is a repeating timer, each timer tick it fires the fdevent object and after 5 ticks it terminates both event sources
//

typedef struct TestCtx_s  {
    int                 callback1_counter;
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    ReactorRef          reactor;
    WTimerFdRef         timer;
    WEventFdRef         fdevent;
    int                 fdevent_counter;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//
// single repeating timer
//

int test_timer_multiple_repeating();
int test_timer_single_repeating();
static void callback_1(WTimerFdRef watcher, void* ctx, XrTimerEvent event);
void fdevent_handler(WEventFdRef fdev_ref, void* arg, uint64_t ev_mask);


int main()
{
    UT_ADD(test_timer_single_repeating);
//    UT_ADD(test_timer_multiple_repeating);
    int rc = UT_RUN();
    return rc;
}
/**
 * Use a time to repeatedly fire a fdevent and check that the
 * fdevent watcher catches all the fired events.
 * All happens in a single thread
 */
#define NBR_TIMES_FIRE 10
int test_timer_single_repeating()
{
    TestCtx* test_ctx_p = TestCtx_new(0, NBR_TIMES_FIRE);

    ReactorRef rtor_ref = XrReactor_new();
    test_ctx_p->reactor = rtor_ref;
    WTimerFdRef tw_1 = WTimerFd_new(rtor_ref, &callback_1, (void*)test_ctx_p, 1000, true);
    WTimerFd_disarm(tw_1);
    WEventFdRef fdev = WEventFd_new(rtor_ref);

    test_ctx_p->fdevent = fdev;
    test_ctx_p->timer = tw_1;


    WEventFd_register(fdev);
    WEventFd_arm(fdev, &fdevent_handler,test_ctx_p);
    WTimerFd_rearm(tw_1);
    XrReactor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    XrReactor_free(rtor_ref);
    return 0;
}

int test_timer_multiple_repeating()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    ReactorRef rtor_ref = XrReactor_new();

    WTimerFdRef tw_1 = WTimerFd_new(rtor_ref, &callback_1, test_ctx_p_1, 100, true);
    WTimerFdRef tw_2 = WTimerFd_new(rtor_ref, &callback_1, test_ctx_p_2, 100, true);

    XrReactor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    XrReactor_free(rtor_ref);
    return 0;
}
static void callback_1(WTimerFdRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    WEventFdRef fdev = ctx_p->fdevent;
    LOG_FMT("callback1_counter %d counter: %d event is : %lx  ", ctx_p->callback1_counter, ctx_p->counter, event);
    if(ctx_p->counter >= ctx_p->max_count) {
        LOG_MSG(" clear timer");
        XrReactor_close(ctx_p->reactor);
//        WTimerFd_clear(watcher);
//        WEventFd_deregister(fdev);
    } else {
        WEventFd_fire(fdev);
        ctx_p->counter++;
        WEventFd_fire(fdev);
        ctx_p->counter++;

    }
    ctx_p->callback1_counter++;
}
void fdevent_handler(WEventFdRef fdev_ref, void* arg, uint64_t ev_mask)
{
    TestCtx* t = (TestCtx*)arg;
    t->fdevent_counter++;
    LOG_FMT("w: %p arg: %p ev mask: %ld fdevent_counter % d", fdev_ref , arg, ev_mask, t->fdevent_counter);
}

TestCtx* TestCtx_new(int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    tmp->counter = counter_init;
    tmp->callback1_counter = 0;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    return tmp;
}
