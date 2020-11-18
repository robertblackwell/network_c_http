#include <c_http/xr/swatcher.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>


static void handler(void* ctx, int fd, uint64_t event)
{
    XrSocketWatcherRef sw = (XrSocketWatcherRef)ctx;
    assert(fd == sw->fd);
    sw->cb(sw, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrSocketWatcherRef twp = (XrSocketWatcherRef)p;
    Xrsw_free(twp);
}
void Xrtw_init(XrSocketWatcherRef this, XrRunloopRef runloop)
{
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
XrSocketWatcherRef Xrtw_new(XrRunloopRef rl)
{
    XrSocketWatcherRef this = malloc(sizeof(XrSocketWatcher));
    Xrtw_init(this, rl);
    return this;
}
void Xrtw_free(XrSocketWatcherRef this)
{
    close(this->fd);
    free((void*)this);
}
void Xrtw_watch(XrSocketWatcherRef this, uint64_t watch_what)
{
    uint32_t interest = watch_what;
    int res = XrRunloop_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrtw_change_watch(XrSocketWatcherRef this, uint64_t watch_what)
{
    uint32_t interest = watch_what;
    int res = XrRunloop_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrtw_clear(XrSocketWatcherRef this)
{
    int res =  XrRunloop_deregister(this->runloop, this->fd);
    assert(res == 0);
}


