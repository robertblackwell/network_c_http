#ifndef C_HTTP_runloop_W_EVENTFD_QUEUE_H
#define C_HTTP_runloop_W_EVENTFD_QUEUE_H

#include "runloop.h"
/** \defgroup eventfdqueue RunloopEventfdQueue
 * @{
 *
 * ## RunloopEventFdQueue and RunloopQueueWatcher
 *
 * A RunloopEventfdQueue is a queue that allows one thread to run a callback function on the
 * runloop of another thread. The queue data structure itself is protected by a mutex.
 * The act of adding an element to the queue causes a file descriptor event to be triggered
 * in the receiver thread so that queue synchronisation interacts well with the other events
 * managed by a runloop.
 *
 * The actual implementation of this mechanism comes in the form of two separate objects and set of functions.
 *
 * A RunloopEventFdQueue is the object implements the queue and it the means by which the sending thread adds to the queue
 * and the receiving thread retrieves items from the queue.
 *
 * A RunloopQueueWatcher object is a file descriptor event object through which the receiver thread is notified
 * that the queue has entries.
 *
 * ### How does it work
 *
 * The way this mechanism works is that the receiving thread:
 *
 * -    creates a runloop,
 * -    creates an EventFdQueue,
 * -    creates a RunloopQueueWatcher,
 * -    shares a pointer to the queue with one or more source threads through either a global var or a thread argument.
 * -    calls runloop_queue_watcher_async_read(qwatcher, on_queue_entry_callback, context_ptr)
 *
 * -    When a source threads adds an entry to the queue by calling eventfd_queue_add(queue, item)
 * -    the on_queue_entry_callback() function will be called with the first item on the queue.
 *
 * ### Receiving thread:
 *
 * ```
 * RunloopRef rl = runloop_new();
 * EventFdQueueRef queue = runloop_eventfd_queue_new()
 * RunloopQueueWatcher qwatcher = runloop_queue_watcher_new(runloop, queue);
 * runloop_queuewatcher_read(qwatcher, on_queue_entry_callback, context_ptr)
 *
 * ....
 *
 * ### The callabck function
 *
 * ```
 * void on_queue_entry_callback(RunloopRef rl, qitem, void* context_ptr)
 * {
 *      // do whatever is appropriate with the item
 * }
 *
 * ```
 *
 * ### Source thread:
 *
 * ....
 *
 * eventfd_queue_add(queue, item);
 *
 * ```
 *
 * ### What are queue entries ?
 *
 * The mechaism described above is __NOT__ a general purpose mechaism for sending data between threads, but rather
 * one that is specilaized for sending a pair of the form (PostableFunction, void* arg) to a runloop.
 *
 * Thus entries that can be exchanged using an EventfdQueue are always and only a datatype called a `Functor`
 * and they are always transfered __BY VALUE__.
 *
 * For an example of this working see test_q_asio and test_q
 *
 */
struct EventfdQueue_s;
typedef struct EventfdQueue_s EventfdQueue, * EventfdQueueRef;

/**
 * Allocate the memory for an EventfdQueue and initialize that memory.
 *
 * @return a EventfdQueueRef
 */
EventfdQueueRef runloop_eventfd_queue_new();
/**
 * Initialize already allocated memory as a valid EventfdQueue
 * @param runloop
 * @param this
 */
void eventfd_queue_init(RunloopRef runloop, EventfdQueueRef this);
/**
 * Deallocate all objects owned by the EventfdQueue including items on the queue
 * but not the memory holding the values used by the eventfdqueue object.
 *
 * @param this
 */
void eventfd_queue_deinit(EventfdQueueRef this);
/**
 * Free all memory associated with `this` EventfdQueue
 * @param athis
 */
void  runloop_eventfd_queue_free(EventfdQueueRef athis);
/**
 * Add a Functor item to an instance of an EventfdQueue.
 *
 *
 * @param athis
 * @param item  Passed in and added to the queue by value
 */
void  runloop_eventfd_queue_add(EventfdQueueRef athis, Functor item);


/**
 * The following 3 functions are aprt of a low level api for eventfd queues.
 */
Functor runloop_eventfd_queue_remove(EventfdQueueRef athis);
int   runloop_eventfd_queue_readfd(EventfdQueueRef athis);
RunloopRef runloop_eventfd_queue_get_runloop(EventfdQueueRef athis);

/** @} */
/** \defgroup queuewatcher RunloopQueueWatcher
 * ## Queue Watcher
 *
 * A runloop_queue_watcher is the mechanism a thread uses to recieve an event whenever
 * a new entry is added to the queue by the same or a different thread.
 */
struct RunloopQueueWatcher_s;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;        // Wait for a inter thread queue event
/**
 * By now you should know how these 4 functions work and how they interact.
 */
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_init(RunloopQueueWatcherRef qw, RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_deinit(RunloopQueueWatcherRef qw);
void runloop_queue_watcher_free(RunloopQueueWatcherRef this);

typedef void(*QueueWatcherReadCallbackFunction)(void* context_ptr, Functor item, int status);
/**
 * This function initiates an asynchronous read of an item from an EventfdQueue via
 * the RunloopQueueWatcher that is associated with the EventfdQueue.
 *
 * When an item is available the callback function will be called
 * @param queue        The queue from which to read an item
 * @param callback     A callback function that will be called when an item is available.
 *                     If status == 0 the read is successfull and the `item` 2nd argument to the
 *                     callback function is valid.
 *                     When status != 0 the second argument is invalid and the read failed.
 * @param context_ptr  This parameter is used by the caller to store any information that will provide
 *                     context that the callback function can use to access the variables that it needs access to.
 */
void runloop_queue_watcher_async_read(RunloopQueueWatcherRef queue_watcher_ref, QueueWatcherReadCallbackFunction callback, void* context_ptr);



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
