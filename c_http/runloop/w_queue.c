#include <c_http/runloop/w_queue.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>


static void handler(WatcherRef watcher, int fd, uint64_t event)
{
    WQueueRef queue_watcher_ref = (WQueueRef)watcher;
    XR_QUEUE_CHECK_TAG(queue_watcher_ref)
    assert(fd == queue_watcher_ref->fd);
    queue_watcher_ref->queue_event_handler(queue_watcher_ref, queue_watcher_ref->queue_event_handler_arg, event);
}
static void anonymous_free(WatcherRef p)
{
    WQueueRef queue_watcher_ref = (WQueueRef)p;
    XR_QUEUE_CHECK_TAG(queue_watcher_ref)
    WQueue_free(queue_watcher_ref);
}
void WQueue_init(WQueueRef this, XrReactorRef runloop, EvfdQueueRef qref)
{
    this->type = XR_WATCHER_QUEUE;
    sprintf(this->tag, "XRQW");
    this->queue = qref;
    this->fd = Evfdq_readfd(qref);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
WQueueRef WQueue_new(XrReactorRef rtor_ref, EvfdQueueRef qref)
{
    WQueueRef this = malloc(sizeof(WQueue));
    WQueue_init(this, rtor_ref, qref);
    return this;
}
void WQueue_free(WQueueRef this)
{
    XRQW_TYPE_CHECK(this)
    XR_QUEUE_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void WQueue_register(WQueueRef this, QueueEventHandler evhandler, void* arg, uint64_t watch_what)
{
    XRQW_TYPE_CHECK(this)
    XR_QUEUE_CHECK_TAG(this)

    uint32_t interest = watch_what;
    this->queue_event_handler = evhandler;
    this->queue_event_handler_arg = arg;
    int res = XrReactor_register(this->runloop, this->fd, interest, (WatcherRef)(this));
    assert(res ==0);
}
void WQueue_change_watch(WQueueRef this, QueueEventHandler evhandler, void* arg, uint64_t watch_what)
{
    XRQW_TYPE_CHECK(this)
    XR_QUEUE_CHECK_TAG(this)
    uint32_t interest = watch_what;
    if( evhandler != NULL) {
        this->queue_event_handler = evhandler;
    }
    if (arg != NULL) {
        this->queue_event_handler_arg = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WQueue_deregister(WQueueRef this)
{
    XRQW_TYPE_CHECK(this)
    XR_QUEUE_CHECK_TAG(this)

    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
