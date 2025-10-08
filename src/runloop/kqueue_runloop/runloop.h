#ifndef C_HTTP_KQ_RUNLOOP_H
#define C_HTTP_KQ_RUNLOOP_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types -= forward declares
///////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Runloop_s Runloop, *RunloopRef;
typedef struct RunloopEvent_s RunloopEvent, *RunloopEventRef;

typedef RunloopEventRef RunloopTimerRef;
typedef RunloopEventRef RunloopListenerRef;
typedef RunloopEventRef RunloopStreamRef;
typedef RunloopEvent RunloopStream;

typedef struct RunloopWatcherBase_s RunloopWatcherBase, *RunloopWatcherBaseRef;       // Base object for objects that wait for an fd event
typedef struct RunloopEventfd_s RunloopEventFd, *RunloopEventFdRef;
// typedef struct AsioStream_s AsioStream, *AsioStreamRef;
// typedef struct AsioListener_s AsioListener, *AsioListenerRef;
typedef struct EventQueue_s EventQueue, * EventQueueRef;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;
/**
 * PostableFunction defines the call signature of functions that can be added to a runloops queue of
 * functions to be called. As such they represent the next step in an ongoing computation of a lightweight
 * "thread".
 */
typedef void (*PostableFunction) (RunloopRef runloop_ref, void* arg);
// typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
// typedef void(*AsioWritecallback)(void* arg, long length, int error_number);
typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functors - not sure why it this promonent
///////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Functor_s
{
    PostableFunction f;
    void *arg;
} Functor, *FunctorRef;

typedef uint64_t EventMask, RunloopTimerEvent;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop interface
///////////////////////////////////////////////////////////////////////////////////////////////////////////

RunloopRef runloop_get_threads_runloop();
RunloopRef runloop_new(void);
void runloop_free(RunloopRef athis);
void runloop_init(RunloopRef athis);
void runloop_deinit(RunloopRef athis);
void runloop_close(RunloopRef athis);
int  runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int  runloop_deregister(RunloopRef athis, int fd);
int  runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int  runloop_run(RunloopRef athis, time_t timeout);
void runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void runloop_delete(RunloopRef athis, int fd);
void runloop_verify(RunloopRef r);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopEventRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopEventRef lrevent, RunloopRef runloop);
void runloop_timer_free(RunloopEventRef lrevent);
void runloop_timer_register(RunloopEventRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopEventRef lrevent, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopEventRef lrevent);
void runloop_timer_rearm_old(RunloopEventRef lrevent, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopEventRef lrevent);
void runloop_timer_deregister(RunloopEventRef lrevent);
RunloopRef runloop_timer_get_runloop(RunloopEventRef lrevent);
/** Convenience interface for timers*/
RunloopEventRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_clear(RunloopRef rl, RunloopEventRef lrevent);
void runloop_timer_checktag(RunloopEventRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runloop Lsitener
///////////////////////////////////////////////////////////////////////////////////////////////////////////

RunloopEventRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopEventRef lrevent);
void runloop_listener_init(RunloopEventRef lrevent, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopEventRef lrevent);
void runloop_listener_register(RunloopEventRef lrevent, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopEventRef lrevent);
void runloop_listener_arm(RunloopEventRef lrevent, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopEventRef lrevent);
void runloop_listener_verify(RunloopEventRef lrevent);
RunloopRef runloop_listener_get_runloop(RunloopEventRef lrevent);
int runloop_listener_get_fd(RunloopEventRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunloopStream
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopEventRef runloop_stream_new(RunloopRef runloop, int fd);
void runloop_stream_free(RunloopEventRef lrevent);
void runloop_stream_init(RunloopEventRef lrevent, RunloopRef runloop, int fd);
void runloop_stream_deinit(RunloopEventRef lrevent);
void runloop_stream_register(RunloopEventRef lrevent);
void runloop_stream_deregister(RunloopEventRef lrevent);
void runloop_stream_arm_both(RunloopEventRef lrevent,
                             PostableFunction read_postable_cb, void* read_arg,
                             PostableFunction write_postable_cb, void* write_arg);

void runloop_stream_arm_read(RunloopEventRef lrevent, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_read(RunloopEventRef lrevent);
void runloop_stream_arm_write(RunloopEventRef lrevent, PostableFunction postable_callback, void* arg);
void runloop_stream_disarm_write(RunloopEventRef lrevent);
void runloop_stream_verify(RunloopStreamRef r);
RunloopRef runloop_stream_get_runloop(RunloopEventRef lrevent);
int runloop_stream_get_fd(RunloopStreamRef this);
void runloop_stream_checktag(RunloopEventRef lrevent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* *
 * kqueue provides a facility to create and wait on an event source that is not attached to any fd/file/pipe/device
 * and to "fire" such events explicitly.
 * 
 * One of the variants of the RunloopEvent struct and related functions use the kqueue facility to provide a
 * generalized mechanism for creating custom events that can be fired and notified
 * using the standard kqueue feature.
 *
 */
RunloopEventRef runloop_user_event_new(RunloopRef runloop);
void runloop_user_event_init(RunloopEventRef athis, RunloopRef runloop);
void runloop_user_event_free(RunloopEventRef athis);
void runloop_user_event_register(RunloopEventRef athis);
void runloop_user_event_change_watch(RunloopEventRef athis, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_user_event_arm(RunloopEventRef athis, PostableFunction postable, void* arg);
void runloop_user_event_disarm(RunloopEventRef athis);
void runloop_user_event_fire(RunloopEventRef athis);
void runloop_user_event_clear_one_event(RunloopEventRef athis);
void runloop_user_event_clear_all_events(RunloopEventRef athis);
void runloop_user_event_deregister(RunloopEventRef athis);
void runloop_user_event_verify(RunloopEventRef r);
RunloopRef runloop_user_event_get_reactor(RunloopEventRef athis);
int runloop_user_event_get_fd(RunloopEventRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Event Queue
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/** 
 *
 * A RunloopEventQueue is a queue that allows one thread to run a callback function on the
 * runloop of another thread. The runloop mechanism provides a queue that is dedicated to inter thread
 * communication and the queue data structure itself is protected by a mutex.
 * 
 * The act of adding an element to the queue causes a user_event to be triggered
 * in the receiver thread so that queue synchronisation interacts well with the other events
 * managed by a runloop.
 *
 * The actual implementation of this mechanism comes in the form of two separate objects and sets of functions.
 *
 * A RunloopEventQueue is the object that implements the queue and it the means by which the sending thread adds to the queue
 * and the receiving thread retrieves items from the queue.
 *
 * A RunloopQueueWatcher object is a user_event object through which the receiver thread is notified
 * that the queue has entries.
 *
 * ### How does it work
 *
 * The way this mechanism works is that the receiving thread:
 *
 * -    create a runloop,
 * -    create a RunloopEventQueue,
 * -    create a RunloopQueueWatcher,
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

/**
 * Allocate the memory for an EventfdQueue and initialize that memory.
 *
 * @return a EventfdQueueRef
 */
EventQueueRef runloop_event_queue_new(RunloopRef runloop, EventQueueRef this);
/**
 * Initialize already allocated memory as a valid EventfdQueue
 * @param runloop
 * @param this
 */
void runloop_event_queue_init(RunloopRef runloop, EventQueueRef this);
/**
 * Deallocate all objects owned by the EventfdQueue including items on the queue
 * but not the memory holding the values used by the eventfdqueue object.
 *
 * @param this
 */
void runloop_event_queue_deinit(EventQueueRef this);
/**
 * Free all memory associated with `this` EventfdQueue
 * @param athis
 */
void  runloop_event_queue_free(EventQueueRef athis);
/**
 * Add a Functor item to an instance of an EventfdQueue.
 *
 *
 * @param athis
 * @param item  Passed in and added to the queue by value
 */
void  runloop_event_queue_add(EventQueueRef athis, Functor item);


/**
 * The following 3 functions are aprt of a low level api for event queues.
 */
Functor runloop_event_queue_remove(EventQueueRef athis);
int   runloop_event_queue_readfd(EventQueueRef athis);
RunloopRef runloop_event_queue_get_runloop(EventQueueRef athis);

/** Queue Watcher
 *
 * A runloop_queue_watcher is the mechanism a thread uses to recieve an event whenever
 * a new entry is added to the queue by the same or a different thread.
 */
/**
 * By now you should know how these 4 functions work and how they interact.
 */
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventQueueRef qref);
void runloop_queue_watcher_init(RunloopQueueWatcherRef qw, RunloopRef runloop, EventQueueRef qref);
void runloop_queue_watcher_deinit(RunloopQueueWatcherRef qw);
void runloop_queue_watcher_free(RunloopQueueWatcherRef this);

typedef void(*QueueWatcherReadCallbackFunction)(void* context_ptr, Functor item, int status);
/**
 * This function initiates an asynchronous read of an item from an EventfdQueue via
 * the RunloopQueueWatcher that is associated with the EventQueue.
 *
 * When an item is available the callback function will be called
 * @param queue        The queue that will generate and event and from which to read an item
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base event
///////////////////////////////////////////////////////////////////////////////////////////////////////////
RunloopRef runloop_watcher_base_get_runloop(RunloopWatcherBaseRef athis);
int        runloop_watcher_base_get_fd(RunloopWatcherBaseRef this);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type safe - these macros provides functions to assert - that is crash if not - the types:
//
// -    RunloopRef
// -    specific subtypes of RunloopEventRef
//      - runloop_listener_verify(p)
//      - runloop_signal_verify(p)
//      - runloop_stream_verify(p)
//      - runloop_timer_verify(p)
//      - runloop_user_event_queue_verify(p)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RUNLOOP_VERIFY(p) runloop_verify(p, __FILE__, __LINE__);
#define RUNLOOP_LISTENER_VERIFY(p) runloop_listener_verify(p, __FILE__, __LINE__);
#define RUNLOOP_SIGNAL_VERIFY(p) runloop_signal_verify(p, __FILE__, __LINE__);
#define RUNLOOP_STREAM_VERIFY(p) runloop_stream_verify(p, __FILE__, __LINE__);
#define RUNLOOP_TIMER_VERIFY(p) runloop_timer_verify(p, __FILE__, __LINE__);
#define RUNLOOP_USER_EVENT_VERIFY(p) runloop_user_event_verify(p, __FILE__, __LINE__);
#define RUNLOOP_USER_EVENT_QUEUE_VERIFY(p) runloop_user_event_queue_verify(p, __FILE__, __LINE__);
#define RUNLOOP_VERIFY(p) runloop_verify(p, __FILE__, __LINE__);

#include "rl_checktag.h"
#include "asio.h"
#endif