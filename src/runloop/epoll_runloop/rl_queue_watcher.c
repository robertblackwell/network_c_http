#include "runloop_internal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

static void on_read_ready_postable(RunloopRef rl, void* qw_arg)
{
    RunloopQueueWatcherRef qw = qw_arg;
    QUEUE_WATCHER_SET_TAG(qw);
    QUEUE_WATCHER_SET_END_TAG(qw)
    Functor fn = runloop_user_event_queue_remove(qw->queue);
    QueueWatcherReadCallbackFunction tmp_cb = qw->read_cb;
    qw->read_cb = NULL;
    void* tmp_arg = qw->read_cb_arg;
    qw->read_cb_arg = NULL;
    runloop_queue_watcher_deregister(qw);
    tmp_cb(tmp_arg, fn, 0);
}
static void handler(RunloopWatcherBaseRef watcher, uint64_t event)
{
    RunloopQueueWatcherRef queue_watcher_ref = (RunloopQueueWatcherRef)watcher;
    QUEUE_WATCHER_CHECK_TAG(queue_watcher_ref)
    QUEUE_WATCHER_CHECK_END_TAG(queue_watcher_ref)
    /**
     * should be posted to runloop not called
     */
    queue_watcher_ref->queue_postable(queue_watcher_ref->runloop, queue_watcher_ref->queue_postable_arg);
}
void runloop_queue_watcher_init(RunloopQueueWatcherRef this, RunloopRef runloop, EventfdQueueRef qref)
{
    QUEUE_WATCHER_SET_TAG(this);
    QUEUE_WATCHER_SET_END_TAG(this)
    this->type = RUNLOOP_WATCHER_QUEUE;
    this->queue = qref;
    this->fd = runloop_user_event_queue_readfd(qref);
    this->runloop = runloop;
    this->handler = &handler;
    this->context = this;
}
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref)
{
    RunloopQueueWatcherRef this = malloc(sizeof(RunloopQueueWatcher));
    runloop_queue_watcher_init(this, runloop, qref);
    return this;
}
void runloop_queue_watcher_deinit(RunloopQueueWatcherRef this)
{
    QUEUE_WATCHER_CHECK_TAG(this)
    QUEUE_WATCHER_CHECK_END_TAG(this)
}
void runloop_queue_watcher_free(RunloopQueueWatcherRef this)
{
    QUEUE_WATCHER_CHECK_TAG(this)
    QUEUE_WATCHER_CHECK_END_TAG(this)
    close(this->fd);
    free(this);
}
void runloop_queue_watcher_async_read(RunloopQueueWatcherRef this, QueueWatcherReadCallbackFunction cb, void* cb_context_arg)
{
    this->read_cb = cb;
    this->read_cb_arg = cb_context_arg;
    runloop_queue_watcher_register(this, on_read_ready_postable, this);
}
void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;

//    uint32_t interest = watch_what;
    athis->queue_postable = postable_cb;
    athis->queue_postable_arg = postable_arg;
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) (athis));
    assert(res ==0);
}
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, PostableFunction cb, void* arg, uint64_t watch_what)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    uint32_t interest = watch_what;
    if(cb != NULL) {
        athis->queue_postable = cb;
    }
    if (arg != NULL) {
        athis->queue_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
void runloop_queue_watcher_deregister(RunloopQueueWatcherRef athis)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)

    int res = runloop_deregister(athis->runloop, athis->fd);
    assert(res == 0);
}
RunloopRef runloop_queue_watcher_get_runloop(RunloopQueueWatcherRef athis)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    return athis->runloop;
}
int runloop_queue_watcher_get_fd(RunloopQueueWatcherRef athis)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    return athis->fd;
}

void runloop_queue_watcher_verify(RunloopQueueWatcherRef athis)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
}
