#include <c_http/macros.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
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
static void handler(RtorWatcherRef watcher, uint64_t event)
{
    struct timespec ts;
    struct itimerspec its;
    struct itimerspec its_old;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    LOG_FMT("XtWatcher::caller current time secs: %ld ns: %ld \n", ts.tv_sec, ts.tv_nsec);
    RtorTimerRef timer_watcher = (RtorTimerRef)watcher;
    XR_WTIMER_CHECK_TAG(timer_watcher)

    int r2 = timerfd_gettime(timer_watcher->fd, &its_old);
    /**
     * Have to read from the fd to reset the EPOLLIN event
     */
    uint64_t ret;
    int nread = read(timer_watcher->fd, &ret, sizeof(ret));
    LOG_FMT("read() returned %d, res=%lx n", nread, ret);

    if(!timer_watcher->repeating) {
        rtor_timer_deregister(timer_watcher);
    }
    CHTTP_ASSERT((timer_watcher->timer_handler != NULL), "timer_handler should not be NULL");
    timer_watcher->timer_handler(timer_watcher, event);
}
static void anonymous_free(RtorWatcherRef p)
{
    RtorTimerRef twp = (RtorTimerRef)p;
    rtor_timer_free(twp);
}
void rtor_timer_init(RtorTimerRef this, ReactorRef runloop)
{
    this->type = XR_WATCHER_TIMER;
    XR_WTIMER_SET_TAG(this)
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->context = NULL;
    this->handler = &handler;
    this->timer_handler = NULL;
    this->timer_handler_arg = NULL;
    this->interval = 0;
    this->repeating = false;
    this->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    LOG_FMT("rtor_timer_init fd: %d \n", this->fd);
}
RtorTimerRef rtor_timer_new(ReactorRef rtor_ref)
{
    RtorTimerRef this = malloc(sizeof(RtorTimer));
    rtor_timer_init(this, rtor_ref);
    return this;
}
void rtor_timer_free(RtorTimerRef athis)
{
    XR_WTIMER_CHECK_TAG(athis)
    close(athis->fd);
    free((void*)athis);
}
struct itimerspec WTimerFd_update_interval(RtorTimerRef this, uint64_t interval_ms, bool repeating)
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
void rtor_timer_register(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(athis)
    athis->interval = interval_ms;
    athis->repeating = repeating;
    athis->timer_handler = cb;
    athis->context = ctx;
    athis->timer_handler_arg = ctx;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    int er = errno;
    assert(rc == 0);
    uint32_t interest = EPOLLIN | EPOLLERR;
    print_current_tme("rtor_timer_register");
    LOG_FMT("rtor_timer_register its.it_value secs %ld nsecs: %ld \n", its.it_value.tv_sec, its.it_value.tv_nsec);
    LOG_FMT("rtor_timer_register its.it_interval secs %ld nsecs: %ld\n", its.it_interval.tv_sec, its.it_interval.tv_nsec);
    int res = rtor_reactor_register(athis->runloop, athis->fd, interest, (RtorWatcherRef) (athis));
    assert(res ==0);
}
void rtor_timer_update(RtorTimerRef athis, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(athis)
    uint32_t interest = 0;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_timer_disarm(RtorTimerRef athis)
{
    XR_WTIMER_CHECK_TAG(athis)
    struct itimerspec its = WTimerFd_update_interval(athis, athis->interval, athis->repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}
void rtor_timer_rearm_old(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    XR_WTIMER_CHECK_TAG(athis)
    athis->repeating = repeating;
    athis->timer_handler = cb;
    athis->timer_handler_arg = ctx;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}
void rtor_timer_rearm(RtorTimerRef athis)
{
    XR_WTIMER_CHECK_TAG(athis)
    uint64_t interval_ms = athis->interval;
    bool repeating = athis->repeating;
    struct itimerspec its = WTimerFd_update_interval(athis, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(athis->fd, flags, &its, NULL);
    assert(rc == 0);
}

void rtor_timer_deregister(RtorTimerRef athis)
{
    XR_WTIMER_CHECK_TAG(athis)
    LOG_FMT("rtor_timer_deregister this->fd : %d\n", athis->fd);
    int res = rtor_reactor_deregister(athis->runloop, athis->fd);
    if(res != 0) {
        LOG_FMT("rtor_timer_deregister res: %d errno: %d \n", res, errno);
    }
    LOG_FMT("rtor_timer_deregister res: %d errno: %d \n", res, errno);
    assert(res == 0);
}
ReactorRef rtor_timer_get_reactor(RtorTimerRef athis)
{
    return athis->runloop;
}
int rtor_timer_get_fd(RtorTimerRef this)
{
    return this->fd;
}
void WTimerFd_verify(RtorTimerRef this)
{
    XR_WTIMER_CHECK_TAG(this)

}
