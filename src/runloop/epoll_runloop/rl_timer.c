#include <rbl/macros.h>
#include <runloop/epoll_runloop/runloop_internal.h>
#include <time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>
#include <rbl/logger.h>
#define TIMER_STATE_REGISTERED 11
#define TIMER_STATE_NOT_REGISTERED 22
#define TIMER_STATE_DISARMED 33

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
static void handler(RunloopWatcherBaseRef watcher, uint64_t event)
{
    struct timespec ts;
    struct itimerspec its;
    struct itimerspec its_old;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    RBL_LOG_FMT("XtWatcher::caller current time secs: %ld ns: %ld", ts.tv_sec, ts.tv_nsec);
    RunloopTimerRef timer_watcher = (RunloopTimerRef)watcher;
    WTIMER_CHECK_TAG(timer_watcher)

    int r2 = timerfd_gettime(timer_watcher->fd, &its_old);
    /**
     * Have to read from the fd to reset the EPOLLIN event
     */
    uint64_t ret;
    int nread = read(timer_watcher->fd, &ret, sizeof(ret));
    RBL_LOG_FMT("read() returned %d, res=%lx n", nread, ret);

    if(!timer_watcher->repeating) {
        runloop_timer_disarm(timer_watcher);
    }
    RBL_ASSERT((timer_watcher->timer_postable != NULL), "timer_handler should not be NULL");
    timer_watcher->timer_postable(timer_watcher->runloop, timer_watcher->timer_postable_arg);
}
static void anonymous_free(RunloopWatcherBaseRef p)
{
    RunloopTimerRef twp = (RunloopTimerRef)p;
    runloop_timer_free(twp);
}
void runloop_timer_init(RunloopTimerRef this, RunloopRef runloop)
{
    this->type = RUNLOOP_WATCHER_TIMER;
    WTIMER_SET_TAG(this)
    WTIMER_SET_END_TAG(this);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->context = NULL;
    this->handler = &handler;
    this->state = TIMER_STATE_NOT_REGISTERED;
    this->timer_postable = NULL;
    this->timer_postable_arg = NULL;
    this->interval = 0;
    this->repeating = false;
    this->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    struct itimerspec its = {.it_value.tv_nsec=0,.it_value.tv_sec=0};//WTimerFd_update_interval(athis, athis->interval, athis->repeating);
    // initialize timer to disarmed and non repeating
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);

    RBL_LOG_FMT("runloop_timer_init fd: %d ", this->fd);
}
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref)
{
    RunloopTimerRef this = rl_event_allocate(runloop_ref, sizeof(RunloopTimer));
    runloop_timer_init(this, runloop_ref);
    return this;
}
void runloop_timer_free(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    close(athis->fd);
    rl_event_free(athis->runloop, athis);
}
struct itimerspec WTimerFd_update_interval(RunloopTimerRef this, uint64_t interval_ms, bool repeating)
{
    WTIMER_CHECK_TAG(this)
    WTIMER_CHECK_END_TAG(this)
    this->repeating = repeating;
    struct timespec ts;
    struct itimerspec its;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    uint32_t interest = 0;
    this->interval = interval_ms;

//    uint64_t ex_time_ns = tns + 1000* interval_ms;
    // make its.it-value and interval rather than an absoluate time
    uint64_t ex_time_ns = 1000* interval_ms;
    its.it_value.tv_sec = (long)ex_time_ns / 1000000;
    its.it_value.tv_nsec = (long)ex_time_ns % 1000000;
    its.it_interval.tv_sec = 11;
    its.it_interval.tv_nsec = 0;

    if (this->repeating) {
        ex_time_ns = 1000* interval_ms;
        its.it_value.tv_sec = (long)ex_time_ns / 1000000;
        its.it_value.tv_nsec = (long)ex_time_ns % 1000000;
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
    } else {
        its.it_interval.tv_sec = 11;
        its.it_interval.tv_nsec = 0;
    }
    return its;
}
void runloop_timer_register(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    RBL_ASSERT((athis != NULL), "");
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    assert(athis->state != TIMER_STATE_REGISTERED);
    athis->state = TIMER_STATE_REGISTERED;
    athis->interval = interval_ms;
    athis->repeating = repeating;
    // interpose our own handler to do repeating stuff
    athis->handler = &handler;
    athis->timer_postable = cb;
    athis->timer_postable_arg = ctx;
    athis->context = ctx;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    int er = errno;
    assert(rc == 0);
    uint32_t interest = EPOLLIN | EPOLLERR;
    print_current_tme("runloop_timer_register");
    RBL_LOG_FMT("runloop_timer_register its.it_value secs %ld nsecs: %ld ", its.it_value.tv_sec, its.it_value.tv_nsec);
    RBL_LOG_FMT("runloop_timer_register its.it_interval secs %ld nsecs: %ld", its.it_interval.tv_sec, its.it_interval.tv_nsec);
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) (athis));
    assert(res ==0);
}
void runloop_timer_update(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    uint32_t interest = 0;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    athis->timer_postable = cb;
    athis->timer_postable_arg = ctx;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}

void runloop_timer_disarm(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    assert(athis->state == TIMER_STATE_REGISTERED);
    athis->state = TIMER_STATE_DISARMED;
    struct itimerspec its = {.it_value.tv_nsec=0,.it_value.tv_sec=0};//WTimerFd_update_interval(athis, athis->interval, athis->repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}
void runloop_timer_rearm_old(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    athis->repeating = repeating;
    athis->timer_postable = cb;
    athis->timer_postable_arg = ctx;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}
void runloop_timer_rearm(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    assert((athis->state == TIMER_STATE_DISARMED)||(athis->state == TIMER_STATE_REGISTERED));
    athis->state = TIMER_STATE_REGISTERED;
    uint64_t interval_ms = athis->interval;
    bool repeating = athis->repeating;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}

void runloop_timer_deregister(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    assert(athis->state != TIMER_STATE_NOT_REGISTERED);
    athis->state = TIMER_STATE_NOT_REGISTERED;
    RBL_LOG_FMT("runloop_timer_deregister this->fd : %d", athis->fd);
    int res = runloop_deregister(athis->runloop, athis->fd);
    if(res != 0) {
        RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    }
    RBL_LOG_FMT("runloop_timer_deregister res: %d errno: %d", res, errno);
    assert(res == 0);
}
RunloopRef runloop_timer_get_runloop(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)
    return athis->runloop;
}
int runloop_timer_get_fd(RunloopTimerRef this)
{
    return this->fd;
}
void WTimerFd_verify(RunloopTimerRef this)
{
    WTIMER_CHECK_TAG(this)
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
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timerref)
{
    runloop_timer_deregister(timerref);
    runloop_timer_free(timerref);
}
void runloop_timer_checktag(RunloopTimerRef athis)
{
    WTIMER_CHECK_TAG(athis)
    WTIMER_CHECK_END_TAG(athis)

}
