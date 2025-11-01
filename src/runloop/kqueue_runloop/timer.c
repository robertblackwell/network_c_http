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
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(RunloopEventRef rlevent, uint16_t event, uint16_t flags)
{
    struct timespec ts;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    RBL_LOG_FMT("runloop_timer::caller current time secs: %ld ns: %ld", ts.tv_sec, ts.tv_nsec);
    RunloopEventRef timer = rlevent;
    TIMER_CHECK_TAG(timer);
    if(!rlevent->timer.repeating) {
        runloop_timer_deregister(timer);
    }
    RBL_ASSERT((rlevent->timer.timer_postable != NULL), "timer_handler should not be NULL");
    rlevent->timer.timer_postable(rlevent->runloop, rlevent->timer.timer_postable_arg);
}
static void anonymous_free(RunloopEventRef lrevent)
{
    RunloopEventRef twp = (RunloopEventRef)lrevent;
    runloop_timer_free(twp);
}
void runloop_timer_init(RunloopEventRef lrevent, RunloopRef runloop)
{
    RunloopEventRef this = (RunloopEventRef)lrevent;
    this->type = RUNLOOP_WATCHER_TIMER;
    TIMER_SET_TAG(this)
    TIMER_SET_END_TAG(this);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->context = NULL;
    this->handler = &handler;
    this->timer.timer_postable = NULL;
    this->timer.timer_postable_arg = NULL;
    this->timer.interval = 0;
    this->timer.repeating = false;
}
RunloopEventRef runloop_timer_new(RunloopRef runloop_ref)
{
    RunloopEventRef this = event_table_get_entry(runloop_ref->event_table);
    runloop_timer_init(this, runloop_ref);
    return this;
}
void runloop_timer_free(RunloopEventRef athis)
{
    TIMER_CHECK_TAG(athis);
    TIMER_CHECK_END_TAG(athis);
    event_table_release_entry(athis->runloop->event_table, athis);
}
void runloop_timer_register(RunloopEventRef rlevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    RBL_ASSERT((rlevent != NULL), "");
    TIMER_CHECK_TAG(rlevent);
    TIMER_CHECK_END_TAG(rlevent);
    rlevent->timer.interval = interval_ms;
    rlevent->timer.repeating = repeating;
    // interpose our own first level handler to do repeating stuff
    rlevent->handler = &handler;
    rlevent->context = ctx;
    rlevent->timer.timer_postable = cb;
    rlevent->timer.timer_postable_arg = ctx;
    int res = kqh_timer_register(rlevent, !repeating, interval_ms);

    print_current_tme("runloop_timer_register");
    assert(res ==0);
}
void runloop_timer_update(RunloopEventRef rlevent, uint64_t interval_ms, bool repeating)
{
    int res = kqh_timer_register(rlevent, !repeating, interval_ms);
    assert(res == 0);
}
void runloop_timer_disarm(RunloopEventRef rlevent)
{
    int res = kqh_timer_pause(rlevent);
    assert(res ==0);
}
void runloop_timer_rearm(RunloopEventRef rlevent)
{
    int res = kqh_timer_register(rlevent, !rlevent->timer.repeating, rlevent->timer.interval);
    assert(res ==0);
}

void runloop_timer_deregister(RunloopEventRef lrevent)
{
    TIMER_CHECK_TAG(lrevent)
    TIMER_CHECK_END_TAG(lrevent)
    int res = kqh_timer_cancel(lrevent);
    if(res != 0) {
        RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    }
    RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    assert(res == 0);
}
RunloopRef runloop_timer_get_runloop(RunloopEventRef lrevent)
{
    TIMER_CHECK_TAG(lrevent);
    TIMER_CHECK_END_TAG(lrevent);
    return lrevent->runloop;
}
void runloop_timer_verify(RunloopEventRef lrevent)
{
    TIMER_CHECK_TAG(lrevent)
    TIMER_CHECK_END_TAG(lrevent);
}
RunloopEventRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    RunloopEventRef tref = runloop_timer_new(rl);
    runloop_timer_register(tref, cb, ctx, interval_ms, repeating);
    return tref;
}
/**
 * After the call to runloop_timer_clear the timerref is invalid and muts not be ised
 */
void runloop_timer_clear(RunloopRef rl, RunloopEventRef lrevent)
{
    TIMER_CHECK_TAG(lrevent)
    TIMER_CHECK_END_TAG(lrevent);
    runloop_timer_deregister(lrevent);
    runloop_timer_free(lrevent);
}
void runloop_timer_checktag(RunloopEventRef lrevent)
{
    TIMER_CHECK_TAG(lrevent)
    TIMER_CHECK_END_TAG(lrevent);
}
