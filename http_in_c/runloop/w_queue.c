#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

static void handler(RunloopWatcherRef watcher, uint64_t event)
{
    RunloopQueueWatcherRef queue_watcher_ref = (RunloopQueueWatcherRef)watcher;
    WQUEUE_CHECK_TAG(queue_watcher_ref)
    /**
     * should be posted to runloop not called
     */
    queue_watcher_ref->queue_postable(queue_watcher_ref->runloop, queue_watcher_ref->queue_postable_arg);
}
static void anonymous_free(RunloopWatcherRef p)
{
    RunloopQueueWatcherRef queue_watcher_ref = (RunloopQueueWatcherRef)p;
    WQUEUE_CHECK_TAG(queue_watcher_ref)
    runloop_queue_watcher_dispose(&queue_watcher_ref);
}
void WQueue_init(RunloopQueueWatcherRef this, RunloopRef runloop, EventfdQueueRef qref)
{
    WQUEUE_SET_TAG(this);
    this->type = RUNLOOP_WATCHER_QUEUE;
    this->queue = qref;
    this->fd = runloop_eventfd_queue_readfd(qref);
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->context = this;
}
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref)
{
    RunloopQueueWatcherRef this = malloc(sizeof(RunloopQueueWatcher));
    WQueue_init(this, runloop, qref);
    return this;
}
void runloop_queue_watcher_dispose(RunloopQueueWatcherRef* athis)
{
    WQUEUE_CHECK_TAG(*athis)
    close((*athis)->fd);
    free((void*)*athis);
    *athis = NULL;
}
void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg)
{
//    WQUEUE_CHECK_TAG(this)
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;

//    uint32_t interest = watch_what;
    athis->queue_postable = postable_cb;
    athis->queue_postable_arg = postable_arg;
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherRef) (athis));
    assert(res ==0);
}
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, QueueEventHandler cb, void* arg, uint64_t watch_what)
{
    WQUEUE_CHECK_TAG(athis)
    uint32_t interest = watch_what;
    if(cb != NULL) {
        athis->queue_postable = cb;
    }
    if (arg != NULL) {
        athis->queue_postable_arg = arg;
    }
    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherRef) athis);
    assert(res == 0);
}
void runloop_queue_watcher_deregister(RunloopQueueWatcherRef athis)
{
    WQUEUE_CHECK_TAG(athis)

    int res = runloop_deregister(athis->runloop, athis->fd);
    assert(res == 0);
}
RunloopRef runloop_queue_watcher_get_reactor(RunloopQueueWatcherRef athis)
{
    return athis->runloop;
}
int runloop_queue_watcher_get_fd(RunloopQueueWatcherRef this)
{
    return this->fd;
}

void runloop_queue_watcher_verify(RunloopQueueWatcherRef r)
{
    WQUEUE_CHECK_TAG(r)

}
