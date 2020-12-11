#include <c_http/xr/queue_watcher.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>


static void handler(XrWatcherRef watcher, int fd, uint64_t event)
{
    XrQueueWatcherRef qw = (XrQueueWatcherRef)watcher;
    assert(fd == qw->fd);
    qw->cb(qw, qw->cb_ctx, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrQueueWatcherRef twp = (XrQueueWatcherRef)p;
    Xrqw_free(twp);
}
void Xrqw_init(XrQueueWatcherRef this, XrReactorRef runloop, EvfdQueueRef qref)
{
    this->type = XR_WATCHER_QUEUE;
    sprintf(this->tag, "XRQW");
    this->queue = qref;
    this->fd = Evfdq_readfd(qref);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
XrQueueWatcherRef Xrqw_new(XrReactorRef rtor_ref, EvfdQueueRef qref)
{
    XrQueueWatcherRef this = malloc(sizeof(XrQueueWatcher));
    Xrqw_init(this, rtor_ref, qref);
    return this;
}
void Xrqw_free(XrQueueWatcherRef this)
{
    XRQW_TYPE_CHECK(this)
    close(this->fd);
    free((void*)this);
}
void Xrqw_register(XrQueueWatcherRef this, XrQueueWatcherCallback cb, void* arg, uint64_t watch_what)
{
    XRQW_TYPE_CHECK(this)

    uint32_t interest = watch_what;
    this->cb = cb;
    this->cb_ctx = arg;
    int res = XrReactor_register(this->runloop, this->fd, interest, (XrWatcherRef)(this));
    assert(res ==0);
}
void Xrqw_change_watch(XrQueueWatcherRef this, XrQueueWatcherCallback cb, void* arg, uint64_t watch_what)
{
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
void Xrqw_deregister(XrQueueWatcherRef this)
{
    XRQW_TYPE_CHECK(this)

    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
