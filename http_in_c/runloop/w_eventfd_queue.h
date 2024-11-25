#ifndef C_HTTP_runloop_W_EVENTFD_QUEUE_H
#define C_HTTP_runloop_W_EVENTFD_QUEUE_H

#include "runloop.h"
/** \defgroup eventfdqueue RunloopEventfdQueue
 * @{
 *
 * ## RunloopEventFdQueue
 *
 * An runloop_eventfd_queue is a queue intended one thread to run a callback function on the
 * runloop of another thread. The queue data structure itself is protected by a mutex.
 * The act of adding an element to the queue causes a file descriptor event to be triggered
 * in the receiver thread so that queue synchronisation interacts well with the other events
 * managed by a runloop.
 */
struct EventfdQueue_s;
typedef struct EventfdQueue_s EventfdQueue, * EventfdQueueRef;

EventfdQueueRef runloop_eventfd_queue_new();
void  runloop_eventfd_queue_free(EventfdQueueRef athis);
int   runloop_eventfd_queue_readfd(EventfdQueueRef athis);
void  runloop_eventfd_queue_add(EventfdQueueRef athis, Functor item);
Functor runloop_eventfd_queue_remove(EventfdQueueRef athis);
//RunloopRef runloop_eventfd_queue_get_runloop(EventfdQueueRef athis);
//#define runloop_eventfd_queue_get_reactor(p) Watcher_get_reactor((RunloopWatcherBaseRef)p)
/** @} */
/** \defgroup queuewatcher RunloopQueueWatcher
 * ## Queue Watcher
 *
 * A runloop_queue_watcher is the mechanism a thread uses to recieve an event whenever
 * a new entry is added to the queue by the same or a different thread.
 */
struct RunloopQueueWatcher_s;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;        // Wait for a inter thread queue event
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_free(RunloopQueueWatcherRef this);
void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg);
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* arg, uint64_t watch_what);
void runloop_queue_watcher_deregister(RunloopQueueWatcherRef athis);
void runloop_queue_watcher_verify(RunloopQueueWatcherRef r);
RunloopRef runloop_queue_watcher_get_reactor(RunloopQueueWatcherRef athis);
int runloop_queue_watcher_get_fd(RunloopQueueWatcherRef this);
//#define runloop_queue_watcher_get_reactor(p) Watcher_get_reactor((RunloopWatcherBaseRef)p)

#if 0
struct InterthreadQueue_s;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;


/**
 *  A RunloopInterthreadQueue is a second implementation of an EventfdQueue with added
 *  functionality (as you can see from the fact that there are more functions).
 *  It is specialized to be part of a Runloop so that other threads can submit
 *  work to a Runloop. An instance of a RunloopInterthreadQueue is used to implement the function
 *
 *  runloop_interthread_post()
 *
 */
InterthreadQueueRef itqueue_new(RunloopRef rl);
void itqueue_free(InterthreadQueueRef this);
void itqueue_add(InterthreadQueueRef qref, Functor func);
Functor itqueue_remove(InterthreadQueueRef qref);
#endif
/** @} */
#endif //C_HTTP_runloop_W_EVENTFD_QUEUE_H
