#include "runloop_internal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void on_read_ready_postable(RunloopRef rl, void* qw_arg)
{
    RunloopQueueWatcherRef qw = qw_arg;
    QUEUE_WATCHER_SET_TAG(qw);
    QUEUE_WATCHER_SET_END_TAG(qw)
    Functor fn = runloop_user_event_queue_remove(qw->queue_watcher.queue);
    QueueWatcherReadCallbackFunction tmp_cb = qw->queue_watcher.read_cb;
    qw->queue_watcher.read_cb = NULL;
    void* tmp_arg = qw->queue_watcher.read_cb_arg;
    qw->queue_watcher.read_cb_arg = NULL;
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
    queue_watcher_ref->queue_watcher.queue_postable(queue_watcher_ref->runloop, queue_watcher_ref->queue_watcher.queue_postable_arg);
}
void runloop_queue_watcher_init(RunloopQueueWatcherRef this, RunloopRef runloop, UserEventQueueRef qref)
{
    QUEUE_WATCHER_SET_TAG(this);
    QUEUE_WATCHER_SET_END_TAG(this)
    this->type = RUNLOOP_WATCHER_QUEUE;
    this->queue_watcher.queue = qref;
    // this->queue_watcher. = runloop_user_event_queue_readfd(qref);
    this->runloop = runloop;
    // this->handler = &handler;
    this->context = this;
}
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, UserEventQueueRef qref)
{
    assert(0);

    RunloopQueueWatcherRef this = NULL;//malloc(sizeof(RunloopQueueWatcher));
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
    // close(this->queue_watcher. fd);
    free(this);
}
void runloop_queue_watcher_async_read(RunloopQueueWatcherRef this, QueueWatcherReadCallbackFunction cb, void* cb_context_arg)
{
    this->queue_watcher.read_cb = cb;
    this->queue_watcher.read_cb_arg = cb_context_arg;
    runloop_queue_watcher_register(this, on_read_ready_postable, this);
}
void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    athis->queue_watcher.queue_postable = postable_cb;
    athis->queue_watcher.queue_postable_arg = postable_arg;
    kqh_user_event_queue_register(athis->queue_watcher.queue);
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) (athis));
    assert(res ==0);
}
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, PostableFunction cb, void* arg, uint64_t watch_what)
{
    QUEUE_WATCHER_CHECK_TAG(athis)
    QUEUE_WATCHER_CHECK_END_TAG(athis)
    uint32_t interest = watch_what;
    if(cb != NULL) {
        athis->queue_watcher.queue_postable = cb;
    }
    if (arg != NULL) {
        athis->queue_watcher.queue_postable_arg = arg;
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
