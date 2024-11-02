#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>
#include <rbl/logger.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void static_handler(RunloopEventfdRef watcher, uint64_t event)
{
    RunloopEventfdRef eventfd_ref = watcher;
    RunloopInterthreadQueueRef itq_ref = (RunloopInterthreadQueueRef) eventfd_ref->fd_event_handler_arg;
    ITQUEUE_CHECK_TAG(itq_ref)
    assert(eventfd_ref == itq_ref->eventfd_ref);
    assert(itq_ref->queue_event_handler != NULL);
    itq_ref->queue_event_handler(itq_ref->queue_event_handler_arg);
}
static void anonymous_free(RunloopWatcherRef p)
{
    RunloopInterthreadQueueRef queue_watcher_ref = (RunloopInterthreadQueueRef)p;
    ITQUEUE_CHECK_TAG(queue_watcher_ref)
    runloop_interthread_queue_dispose(queue_watcher_ref);
}
void runloop_interthread_queue_init(RunloopInterthreadQueueRef this, RunloopRef runloop)
{
    ITQUEUE_SET_TAG(this);
    this->eventfd_ref = runloop_eventfd_new(runloop);
    this->queue = RunList_new();
//    this->queue_event_handler = NULL;
//    this->queue_event_handler_arg = NULL;
}
RunloopInterthreadQueueRef runloop_interthread_queue_new(RunloopRef runloop_ref)
{
    RunloopInterthreadQueueRef this = malloc(sizeof(RunloopInterthreadQueue ));
    runloop_interthread_queue_init(this, runloop_ref);
    return this;
}
void runloop_interthread_queue_dispose(RunloopInterthreadQueueRef this)
{
    ITQUEUE_CHECK_TAG(this)
    close(this->eventfd_ref->fd);
    free((void*)this);
}
void runloop_interthread_queue_add(RunloopInterthreadQueueRef this, void* item)
{
    ITQUEUE_CHECK_TAG(this)
    pthread_mutex_lock(&(this->queue_mutex));
    List_add_back(this->queue, item);
    RBL_LOG_FMT("Queue_add: %d\n", List_size(me->list));
//    runloop_eventfd_fire(this->eventfd_ref);
    uint64_t buf = 1;
    int x = write(this->eventfd_ref->write_fd, &buf, sizeof(buf));
    assert(x == sizeof(buf));

    pthread_mutex_unlock(&(this->queue_mutex));

}
void runloop_interthread_queue_drain(RunloopInterthreadQueueRef this, void(*draincb)(void*))
{
    ITQUEUE_CHECK_TAG(this)
    RunloopRef rx = this->eventfd_ref->runloop;
    pthread_mutex_lock(&(this->queue_mutex));
    for(void* op = List_remove_first(this->queue); op != NULL;) {
        draincb(op);
    }
    runloop_eventfd_clear_all_events(this->eventfd_ref);
    pthread_mutex_unlock(&(this->queue_mutex));
    RBL_LOG_FMT("Queue_pop: socket is %ld nread: %d buf : %ld\n", (long)op, nread, buf);
    // remember to read from the pipe to clear the event
}
void runloop_interthread_queue_register(RunloopInterthreadQueueRef this, InterthreadQueueEventHandler evhandler, void* arg, uint64_t watch_what)
{
    ITQUEUE_CHECK_TAG(this)

    uint32_t interest = watch_what;
    void* eventfd_ptr = this->eventfd_ref;
    void* queue_handler_ptr = &(this->queue_event_handler);
    void* queue_handler_arg = &(this->queue_event_handler_arg);
    this->queue_event_handler = evhandler;
    this->queue_event_handler_arg = arg;
    this->eventfd_ref->fd_event_handler = static_handler;
    this->eventfd_ref->fd_event_handler_arg = (void*) this;
    runloop_eventfd_register(this->eventfd_ref);
//    int res = runloop_register(this->runloop, this->fd, interest, (RunloopWatcherRef) (this));
//    assert(res ==0);
}
void runloop_interthreaD_queue_deregister(RunloopInterthreadQueueRef this)
{
    ITQUEUE_CHECK_TAG(this)
    int res = runloop_deregister(this->eventfd_ref->runloop, this->eventfd_ref->fd);
    assert(res == 0);
}
RunloopRef runloop_interthread_queue_get_reactor(RunloopInterthreadQueueRef this)
{
    ITQUEUE_CHECK_TAG(this)
    return this->eventfd_ref->runloop;
}
int runloop_interthread_queue_get_fd(RunloopInterthreadQueueRef this)
{
    ITQUEUE_CHECK_TAG(this)
    return this->eventfd_ref->fd;
}

void runloop_interthread_queue_verify(RunloopInterthreadQueueRef this)
{
    ITQUEUE_CHECK_TAG(this)
}
