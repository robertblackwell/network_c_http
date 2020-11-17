#include <time.h>

#include <sys/timerfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/epoll.h>
#include <c_http/xr/twatcher.h>

static void caller(void* ctx, int fd, uint64_t event)
{
    XrTimerWatcherRef tw = (XrTimerWatcherRef)ctx;
    assert(fd == tw->fd);
    tw->cb(tw, event);
}
static void anaonymous_free(XrWatcherRef p)
{
    XrTimerWatcherRef twp = (XrTimerWatcherRef)p;
    Xrtw_free(twp);
}
void Xrtw_init(XrTimerWatcherRef this, XrRunloopRef runloop)
{
    this->runloop = runloop;
    this->free = &anaonymous_free;
    this->handler = &caller;
    this->repeating = false;
    this->fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
}
XrTimerWatcherRef Xrtw_new(XrRunloopRef rl)
{
    XrTimerWatcherRef this = malloc(sizeof(XrTimerWatcher));
    Xrtw_init(this, rl);
    return this;
}
void Xrtw_free(XrTimerWatcherRef this)
{
    close(this->fd);
    free((void*)this);
}
struct itimerspec Xrtw_update_interval(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating)
{
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

    if (this->repeating) {
        uint64_t ex_time_ns = 1000* interval_ms;
        its.it_value.tv_sec = ex_time_ns / 1000000;
        its.it_value.tv_nsec = ex_time_ns % 1000000;
    } else {
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 0;
    }
    return its;

}
void Xrtw_set(XrTimerWatcherRef this, XrTimerWatcherCallback cb, uint64_t interval_ms, bool repeating)
{
    this->repeating = repeating;
    this->cb = cb;
    struct itimerspec its = Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    assert(rc == 0);
    uint32_t interest = EPOLLIN | EPOLLERR;
    int res = XrRunloop_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrtw_update(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating)
{
    uint32_t interest = 0;
    struct itimerspec its =Xrtw_update_interval(this, interval_ms, repeating);
    int flags = 0;
    int rc = timerfd_settime(this->fd, flags, &its, NULL);
    int res = XrRunloop_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrtw_clear(XrTimerWatcherRef this)
{
    int res =  XrRunloop_deregister(this->runloop, this->fd);
    assert(res == 0);
}


