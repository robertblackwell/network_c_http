#include <rbl/macros.h>
#include "runloop_internal.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <rbl/logger.h>

static struct timespec current_time()
{
    struct timespec ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}
static void print_current_tme(char* prefix)
{
    struct timespec ts = current_time();
    RBL_LOG_FMT("%s current time secs: %ld ns: %ld ", prefix, ts.tv_sec, ts.tv_nsec);
}
/**
 * First level fd event handler - provided in the base/common part of an event source
 * object. Called directly from the select/epoll_wait loop
 */
static void handler(RunloopWatcherBaseRef watcher, uint16_t event, uint16_t flags, void* data)
{
    RunloopTimerRef timer = (RunloopTimerRef)watcher;
    TIMER_CHECK_TAG(timer)
    TIMER_CHECK_END_TAG(timer)
    struct timespec ts;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    RBL_LOG_FMT("runloop_timer::caller current time secs: %ld ns: %ld", ts.tv_sec, ts.tv_nsec);
    TIMER_CHECK_TAG(timer);
    if(!timer->repeating) {
        runloop_timer_deregister(timer);
    }
    RBL_ASSERT((timer->timer_postable != NULL), "timer_handler should not be NULL");
    timer->timer_postable(timer->runloop, timer->timer_postable_arg);
}
void runloop_timer_init(RunloopTimerRef timer, RunloopRef runloop)
{
    RunloopTimerRef this = (RunloopTimerRef)timer;
    this->type = RUNLOOP_WATCHER_TIMER;
    TIMER_SET_TAG(this)
    TIMER_SET_END_TAG(this);
    this->runloop = runloop;
    this->context = NULL;
    this->handler = &handler;
    this->timer_postable = NULL;
    this->timer_postable_arg = NULL;
    this->interval = 0;
    this->repeating = false;
}
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref)
{
    RunloopTimerRef this = runloop_event_allocate(runloop_ref, sizeof(RunloopTimer));
    runloop_timer_init(this, runloop_ref);
    return this;
}
void runloop_timer_free(RunloopTimerRef athis)
{
    TIMER_CHECK_TAG(athis);
    TIMER_CHECK_END_TAG(athis);
    runloop_event_free(athis->runloop, athis);
}
void runloop_timer_register(RunloopTimerRef timer, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    RBL_ASSERT((timer != NULL), "");
    TIMER_CHECK_TAG(timer);
    TIMER_CHECK_END_TAG(timer);
    timer->interval = interval_ms;
    timer->repeating = repeating;
    // interpose our own first level handler to do repeating stuff
    timer->handler = &handler;
    timer->context = ctx;
    timer->timer_postable = cb;
    timer->timer_postable_arg = ctx;
    int res = kqh_timer_register(timer, !repeating, interval_ms);

    print_current_tme("runloop_timer_register");
    assert(res ==0);
}
void runloop_timer_update(RunloopTimerRef timer, uint64_t interval_ms, bool repeating)
{
    int res = kqh_timer_register(timer, !repeating, interval_ms);
    assert(res == 0);
}
void runloop_timer_disarm(RunloopTimerRef timer)
{
    int res = kqh_timer_pause(timer);
    assert(res ==0);
}
void runloop_timer_rearm(RunloopTimerRef timer)
{
    int res = kqh_timer_register(timer, !timer->repeating, timer->interval);
    assert(res ==0);
}

void runloop_timer_deregister(RunloopTimerRef timer)
{
    TIMER_CHECK_TAG(timer)
    TIMER_CHECK_END_TAG(timer)
    int res = kqh_timer_cancel(timer);
    if(res != 0) {
        RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    }
    RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    assert(res == 0);
}
RunloopRef runloop_timer_get_runloop(RunloopTimerRef timer)
{
    TIMER_CHECK_TAG(timer);
    TIMER_CHECK_END_TAG(timer);
    return timer->runloop;
}
void runloop_timer_verify(RunloopTimerRef timer)
{
    TIMER_CHECK_TAG(timer)
    TIMER_CHECK_END_TAG(timer);
}
RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    RunloopTimerRef tref = runloop_timer_new(rl);
    runloop_timer_register(tref, cb, ctx, interval_ms, repeating);
    return tref;
}
/**
 * After the call to runloop_timer_clear the timerref is invalid and muts not be ised
 */
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timer)
{
    TIMER_CHECK_TAG(timer)
    TIMER_CHECK_END_TAG(timer);
    runloop_timer_deregister(timer);
    runloop_timer_free(timer);
}
void runloop_timer_checktag(RunloopTimerRef timer)
{
    TIMER_CHECK_TAG(timer)
    TIMER_CHECK_END_TAG(timer);
}
