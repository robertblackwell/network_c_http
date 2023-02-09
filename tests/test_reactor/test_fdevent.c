#define _GNU_SOURCE
#define ENABLE_LOGx
#include <http_in_c/async-old/types.h>
#include <stdio.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <http_in_c/logger.h>
#include <http_in_c/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
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
    RtorTimerRef         timer;
    RtorEventfdRef         fdevent;
    int                 fdevent_counter;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max);

//
// single repeating timer
//

int test_fdevent_multiple();
int test_fdevent_1();
static void callback_1(RtorTimerRef watcher, XrTimerEvent event);
void fdevent_handler(RtorEventfdRef fdev_ref, uint64_t ev_mask);


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

    ReactorRef rtor_ref = rtor_reactor_new();
    test_ctx_p->reactor = rtor_ref;
    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_1, (void *) test_ctx_p, 1000, true);
    rtor_timer_disarm(tw_1);
    RtorEventfdRef fdev = rtor_eventfd_new(rtor_ref);

    test_ctx_p->fdevent = fdev;
    test_ctx_p->timer = tw_1;


    rtor_eventfd_register(fdev);
    rtor_eventfd_arm(fdev, &fdevent_handler, test_ctx_p);
    rtor_timer_rearm(tw_1);
    rtor_reactor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    rtor_reactor_free(rtor_ref);
    return 0;
}

int test_fdevent_multiple()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    ReactorRef rtor_ref = rtor_reactor_new();

    RtorTimerRef tw_1 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_1, &callback_1, test_ctx_p_1, 100, true);

    RtorTimerRef tw_2 = rtor_timer_new(rtor_ref);
    rtor_timer_register(tw_2, &callback_1, test_ctx_p_2, 100, true);

    rtor_reactor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    rtor_reactor_free(rtor_ref);
    return 0;
}
static void callback_1(RtorTimerRef watcher, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    void* ctx = watcher->timer_handler_arg;
    TestCtx* ctx_p = (TestCtx*) ctx;
    RtorEventfdRef fdev = ctx_p->fdevent;
    LOG_FMT("callback1_counter %d counter: %d event is : %lx  ", ctx_p->callback1_counter, ctx_p->counter, event);
    if(ctx_p->counter >= ctx_p->max_count) {
        LOG_MSG(" clear timer");
        rtor_reactor_close(ctx_p->reactor);
//        rtor_timer_deregister(watcher);
//        rtor_eventfd_deregister(fdev);
    } else {
        rtor_eventfd_fire(fdev);
        ctx_p->counter++;
        rtor_eventfd_fire(fdev);
        ctx_p->counter++;

    }
    ctx_p->callback1_counter++;
}
void fdevent_handler(RtorEventfdRef fdev_ref, uint64_t ev_mask)
{
    void* arg = fdev_ref->fd_event_handler_arg;
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
