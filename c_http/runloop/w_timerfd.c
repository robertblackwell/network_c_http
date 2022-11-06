#include <c_http/runloop/w_timerfd.h>
#include <time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>
#include <c_http/logger.h>

static struct timespec current_time()
{
    struct timespec ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}
static void print_current_tme(char* prefix)
{
    struct timespec ts = current_time();
    LOG_FMT("%s current time secs: %ld ns: %ld \n", prefix, ts.tv_sec, ts.tv_nsec);
}
/**
 * First level fd event handler - provided in the base/common part of an event source
 * object. Called directly from the select/epoll_wait loop
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(WatcherRef watcher, int fd, uint64_t event)
{
    struct timespec ts;
    struct itimerspec its;
    struct itimerspec its_old;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    LOG_FMT("XtWatcher::caller current time secs: %ld ns: %ld \n", ts.tv_sec, ts.tv_nsec);
    WTimerFdRef timer_watcher = (WTimerFdRef)watcher;
    XR_WTIMER_CHECK_TAG(timer_watcher)

    int r2 = timerfd_gettime(timer_watcher->fd, &its_old);
    assert(fd == timer_watcher->fd);
    /**
     * Have to read from the fd to reset the EPOLLIN event
     */
    uint64_t ret;
    int nread = read(timer_watcher->fd, &ret, sizeof(ret));
    LOG_FMT("read() returned %d, res=%lx n", nread, ret);

    if(!timer_watcher->repeating) {
        WTimerFd_clear(timer_watcher);
    }
    timer_watcher->timer_handler(timer_watcher, timer_watcher->timer_handler_arg, event);
}
static void anonymous_free(WatcherRef p)
{
    WTimerFdRef twp = (WTimerFdRef)p;
    WTimerFd_free(twp);
}
void WTimerFd_init(WTimerFdRef this, ReactorRef runloop, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    this->type = XR_WATCHER_TIMER;
    XR_WTIMER_SET_TAG(this)
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->repeating = false;
    this->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    WTimerFd_set(this, cb, ctx, interval_ms, repeating);
    LOG_FMT("WTimerFd_init fd: %d \n", this->fd);
}
WTimerFdRef WTimerFd_new(ReactorRef rtor_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    WTimerFdRef this = malloc(sizeof(WTimer));
    WTimerFd_init(this, rtor_ref, cb, ctx, interval_ms, repeating);
    return this;
}
void WTimerFd_free(WTimerFdRef this)
{
    XR_WTIMER_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
struct itimerspec WTimerFd_update_interval(WTimerFdRef this, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(this)
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
    its.it_value.tv_sec = ex_time_ns / 1000000;
    its.it_value.tv_nsec = ex_time_ns % 1000000;
    its.it_interval.tv_sec = 11;
    its.it_interval.tv_nsec = 0;

    if (this->repeating) {
        uint64_t ex_time_ns = 1000* interval_ms;
        its.it_value.tv_sec = ex_time_ns / 1000000;
        its.it_value.tv_nsec = ex_time_ns % 1000000;
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
    } else {
        its.it_interval.tv_sec = 11;
        its.it_interval.tv_nsec = 0;
    }
    return its;
}
void WTimerFd_set(WTimerFdRef this, TimerEventHandler eventHandler, void* arg, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(this)
    this->interval = interval_ms;
    this->repeating = repeating;
    this->timer_handler = eventHandler;
    this->timer_handler_arg = arg;
    struct itimerspec its = WTimerFd_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    int er = errno;
    assert(rc == 0);
    uint32_t interest = EPOLLIN | EPOLLERR;
    print_current_tme("WTimerFd_set");
    LOG_FMT("WTimerFd_set its.it_value secs %ld nsecs: %ld \n", its.it_value.tv_sec, its.it_value.tv_nsec);
    LOG_FMT("WTimerFd_set its.it_interval secs %ld nsecs: %ld\n", its.it_interval.tv_sec, its.it_interval.tv_nsec);
    int res = XrReactor_register(this->runloop, this->fd, interest, (WatcherRef)(this));
    assert(res ==0);
}
void WTimerFd_update(WTimerFdRef this, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(this)
    uint32_t interest = 0;
    struct itimerspec its = WTimerFd_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WTimerFd_disarm(WTimerFdRef this)
{
    XR_WTIMER_CHECK_TAG(this)
    struct itimerspec its = WTimerFd_update_interval(this, this->interval, this->repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}
void WTimerFd_rearm_old(WTimerFdRef this, TimerEventHandler eventHandler, void* arg, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(this)
    this->repeating = repeating;
    this->timer_handler = eventHandler;
    this->timer_handler_arg = arg;
    struct itimerspec its = WTimerFd_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}
void WTimerFd_rearm(WTimerFdRef this)
{
    XR_WTIMER_CHECK_TAG(this)
    uint64_t interval_ms = this->interval;
    bool repeating = this->repeating;
    struct itimerspec its = WTimerFd_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}

void WTimerFd_clear(WTimerFdRef this)
{
    XR_WTIMER_CHECK_TAG(this)
    LOG_FMT("WTimerFd_clear this->fd : %d\n", this->fd);
    int res =  XrReactor_deregister(this->runloop, this->fd);
    if(res != 0) {
        LOG_FMT("WTimerFd_clear res: %d errno: %d \n", res, errno);
    }
    LOG_FMT("WTimerFd_clear res: %d errno: %d \n", res, errno);
    assert(res == 0);
}
