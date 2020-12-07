#include <c_http/xr/socket_watcher.h>
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
    sw->cb((XrWatcherRef)sw, sw->cb_ctx, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrSocketWatcherRef twp = (XrSocketWatcherRef)p;
    Xrsw_free(twp);
}
void Xrsw_init(XrSocketWatcherRef this, XrReactorRef runloop, int fd)
{
    this->type = XR_WATCHER_SOCKET;
    sprintf(this->tag, "XRSW");
    XR_SOCKW_SET_TAG(this);
    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
XrSocketWatcherRef Xrsw_new(XrReactorRef rtor_ref, int fd)
{
    XrSocketWatcherRef this = malloc(sizeof(XrSocketWatcher));
    Xrsw_init(this, rtor_ref, fd);
    return this;
}
void Xrsw_free(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void Xrsw_register(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)

    uint32_t interest = watch_what;
    this->cb = cb;
    this->cb_ctx = arg;
    int res = XrReactor_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrsw_change_watch(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what)
{
    XR_SOCKW_CHECK_TAG(this)
    uint32_t interest = watch_what;
    if( cb != NULL) {
        this->cb = cb;
    }
    if (arg != NULL) {
        this->cb_ctx = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrsw_deregister(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)

    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}


