#include <time.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>

#include <c_http/xr/timer_watcher.h>



static struct timespec current_time()
{
    struct timespec ts;
    int r = clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}
static void print_current_tme(char* prefix)
{
    struct timespec ts = current_time();
    XR_PRINTF("%s current time secs: %ld ns: %ld \n", prefix, ts.tv_sec, ts.tv_nsec);
}
static void caller(void* ctx, int fd, uint64_t event)
{
    struct timespec ts;
    struct itimerspec its;
    struct itimerspec its_old;

    int r = clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t tns = ts.tv_sec * 1000000 + ts.tv_nsec;
    XR_PRINTF("XtWatcher::caller current time secs: %ld ns: %ld \n", ts.tv_sec, ts.tv_nsec);
    XrTimerWatcherRef tw = (XrTimerWatcherRef)ctx;
    int r2 = timerfd_gettime(tw->fd, &its_old);
    assert(fd == tw->fd);
    /**
     * Have to read from the fd to reset the EPOLLIN event
     */
    uint64_t ret;
    int nread = read(tw->fd, &ret, sizeof(ret));
    XR_PRINTF("read() returned %d, res=%lx n", nread, ret);

    if(!tw->repeating) {
        Xrtw_clear(tw);
    }
    tw->cb(tw, tw->cb_ctx, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrTimerWatcherRef twp = (XrTimerWatcherRef)p;
    Xrtw_free(twp);
}
void Xrtw_init(XrTimerWatcherRef this, XrReactorRef runloop)
{
    this->type = XR_WATCHER_TIMER;
    sprintf(this->tag, "XRTW");
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &caller;
    this->repeating = false;
    this->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
    XR_PRINTF("Xrtw_init fd: %d \n", this->fd);
}
XrTimerWatcherRef Xrtw_new(XrReactorRef rtor_ref)
{
    XrTimerWatcherRef this = malloc(sizeof(XrTimerWatcher));
    Xrtw_init(this, rtor_ref);
    return this;
}
void Xrtw_free(XrTimerWatcherRef this)
{
    XRTW_TYPE_CHECK(this)
    close(this->fd);
    free((void*)this);
}
struct itimerspec Xrtw_update_interval(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating)
{
    XRTW_TYPE_CHECK(this)
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
void Xrtw_set(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    XRTW_TYPE_CHECK(this)
    this->interval = interval_ms;
    this->repeating = repeating;
    this->cb = cb;
    this->cb_ctx = ctx;
    struct itimerspec its = Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    int er = errno;
    assert(rc == 0);
    uint32_t interest = EPOLLIN | EPOLLERR;
    print_current_tme("Xrtw_set");
    XR_PRINTF("Xrtw_set its.it_value secs %ld nsecs: %ld \n", its.it_value.tv_sec, its.it_value.tv_nsec);
    XR_PRINTF("Xrtw_set its.it_interval secs %ld nsecs: %ld\n", its.it_interval.tv_sec, its.it_interval.tv_nsec);
    int res = XrReactor_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrtw_update(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating)
{
    XRTW_TYPE_CHECK(this)
    uint32_t interest = 0;
    struct itimerspec its = Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrtw_disarm(XrTimerWatcherRef this)
{
    XRTW_TYPE_CHECK(this)
    struct itimerspec its = Xrtw_update_interval(this, this->interval, this->repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}
void Xrtw_rearm(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating)
{
    XRTW_TYPE_CHECK(this)
    this->repeating = repeating;
    this->cb = cb;
    this->cb_ctx = ctx;
    struct itimerspec its = Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}
void Xrtw_rearm_2(XrTimerWatcherRef this)
{
    XRTW_TYPE_CHECK(this)
    uint64_t interval_ms = this->interval;
    bool repeating = this->repeating;
    struct itimerspec its = Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
}

void Xrtw_clear(XrTimerWatcherRef this)
{
    XRTW_TYPE_CHECK(this)
    XR_PRINTF("Xrtw_clear this->fd : %d\n", this->fd);
    int res =  XrReactor_deregister(this->runloop, this->fd);
    if(res != 0) {
        XR_PRINTF("Xrtw_clear res: %d errno: %d \n", res, errno);
    }
    XR_PRINTF("Xrtw_clear res: %d errno: %d \n", res, errno);
    assert(res == 0);
}


