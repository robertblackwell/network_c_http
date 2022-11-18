#include <c_http/simple_runloop/runloop.h>
#include <c_http//simple_runloop/rl_internal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

static void handler(RtorWatcherRef watcher, uint64_t event)
{
    WQueueRef queue_watcher_ref = (WQueueRef)watcher;
    XR_WQUEUE_CHECK_TAG(queue_watcher_ref)
    queue_watcher_ref->queue_event_handler(queue_watcher_ref, event);
}
static void anonymous_free(RtorWatcherRef p)
{
    WQueueRef queue_watcher_ref = (WQueueRef)p;
    XR_WQUEUE_CHECK_TAG(queue_watcher_ref)
    WQueue_dispose(queue_watcher_ref);
}
void WQueue_init(WQueueRef this, ReactorRef runloop, EvfdQueueRef qref)
{
    XR_WQUEUE_SET_TAG(this);
    this->type = XR_WATCHER_QUEUE;
    this->queue = qref;
    this->fd = Evfdq_readfd(qref);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
WQueueRef WQueue_new(ReactorRef rtor_ref, EvfdQueueRef qref)
{
    WQueueRef this = malloc(sizeof(WQueue));
    WQueue_init(this, rtor_ref, qref);
    return this;
}
void WQueue_dispose(WQueueRef this)
{
    XR_WQUEUE_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void WQueue_register(WQueueRef this, QueueEventHandler evhandler, void* arg, uint64_t watch_what)
{
//    XR_WQUEUE_CHECK_TAG(this)

    uint32_t interest = watch_what;
    this->queue_event_handler = evhandler;
    this->queue_event_handler_arg = arg;
    int res = rtor_register(this->runloop, this->fd, interest, (RtorWatcherRef) (this));
    assert(res ==0);
}
void WQueue_change_watch(WQueueRef this, QueueEventHandler evhandler, void* arg, uint64_t watch_what)
{
    XR_WQUEUE_CHECK_TAG(this)
    uint32_t interest = watch_what;
    if( evhandler != NULL) {
        this->queue_event_handler = evhandler;
    }
    if (arg != NULL) {
        this->queue_event_handler_arg = arg;
    }
    int res = rtor_reregister(this->runloop, this->fd, interest, (RtorWatcherRef) this);
    assert(res == 0);
}
void WQueue_deregister(WQueueRef this)
{
    XR_WQUEUE_CHECK_TAG(this)

    int res = rtor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
ReactorRef WQueue_get_reactor(WQueueRef athis)
{
    return athis->runloop;
}
int WQueue_get_fd(WQueueRef athis)
{
    return athis->fd;
}

void WQueue_verify(WQueueRef this)
{
    XR_WQUEUE_CHECK_TAG(this)

}
