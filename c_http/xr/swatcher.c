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
    sw->cb(sw, sw->cb_ctx, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrSocketWatcherRef twp = (XrSocketWatcherRef)p;
    Xrsw_free(twp);
}
void Xrsw_init(XrSocketWatcherRef this, XrRunloopRef runloop, int fd)
{
    this->type = XR_WATCHER_SOCKET;
    sprintf(this->tag, "XRSW");

    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
XrSocketWatcherRef Xrsw_new(XrRunloopRef rl, int fd)
{
    XrSocketWatcherRef this = malloc(sizeof(XrSocketWatcher));
    Xrsw_init(this, rl, fd);
    return this;
}
void Xrsw_free(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    close(this->fd);
    free((void*)this);
}
void Xrsw_register(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what)
{
    XRSW_TYPE_CHECK(this)

    uint32_t interest = watch_what;
    this->cb = cb;
    this->cb_ctx = arg;
    int res = XrRunloop_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrsw_change_watch(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what)
{
    uint32_t interest = watch_what;
    if( cb != NULL) {
        this->cb = cb;
    }
    if (arg != NULL) {
        this->cb_ctx = arg;
    }
    int res = XrRunloop_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrsw_deregister(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)

    int res =  XrRunloop_deregister(this->runloop, this->fd);
    assert(res == 0);
}


