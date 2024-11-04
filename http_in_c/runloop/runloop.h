#ifndef C_HTTP_RUNLOOP_H
#define C_HTTP_RUNLOOP_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Opaque objects
 */
struct Runloop_s;
typedef struct Runloop_s Runloop, *RunloopRef;

struct RunloopWatcher_s;
typedef struct RunloopWatcher_s RunloopWatcher, *RunloopWatcherRef;       // Base object for objects that wait for an fd event

/**
 * There are 5 types of events that can be waited for. They are:
 * -    timer, wait for it to expire
 * -    socket/pipe,  ready for read, ready for write
 * -    socket, ready for accept() call
 * -    a linux eventfd, wait for the fd to be triggered
 * -    interthread queue, wait for an entry to be added
 *
 * Each type of event requires a specific type of opaque object
 * in order to perform such an event wait
 */
struct RunloopTimer_s;
typedef struct RunloopTimer_s RunloopTimer,  *RunloopTimerRef;      // Wait for a timer event

struct RunloopStream_s;
typedef struct RunloopStream_s RunloopStream, *RunloopStreamRef;         // Wait for an IO event on a fd

struct RunloopQueueWatcher_s;
typedef struct RunloopQueueWatcher_s RunloopQueueWatcher, *RunloopQueueWatcherRef;        // Wait for a inter thread queue event

struct RunloopListener_s;
typedef struct RunloopListener_s RunloopListener, * RunloopListenerRef;   // Special event for socket listen()

struct RunloopEventfd_s;
typedef struct RunloopEventfd_s RunloopEventfd, *RunloopEventfdRef;      // Waiter for epoll event fd event

struct EventfdQueue_s;
typedef struct EventfdQueue_s EventfdQueue, * EventfdQueueRef;

//struct RunloopInterthreadQueue_s;
//typedef struct RunloopInterthreadQueue_s RunloopInterthreadQueue, *RunloopInterthreadQueueRef;        // Wait for a inter thread queue event

struct Functor_s;
typedef struct Functor_s Functor;

struct InterthreadQueue_s;
typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;


typedef uint64_t EventMask, RunloopTimerEvent;

// A generic callback function
typedef void (*PostableFunction)    (RunloopRef runloop_ref, void* arg);
// Signature of functions that can called by the Runloop to handle file descriptor events
typedef void (*WatcherCallback)     (RunloopWatcherRef wref, uint64_t events);
// Type specific event handlers - these are all the same except for the casting of the first arg to a specific type of pointer
typedef void (*WatcherEventHandler) (RunloopWatcherRef wref, uint64_t events);
typedef void (*TimerEventHandler)   (RunloopTimerRef timer_watcher_ref, uint64_t events);
typedef void (*SocketEventHandler)  (RunloopStreamRef socket_watcher_ref, uint64_t events);
typedef void (*FdEventHandler)      (RunloopEventfdRef fd_event_ref, uint64_t events);
typedef void (*QueueEventHandler)   (RunloopQueueWatcherRef qref, uint64_t events);
//typedef void (*InterthreadQueueEventHandler)   (RunloopInterthreadQueueRef qref);
typedef void (*ListenerEventHandler)(RunloopListenerRef listener_ref, uint64_t events);

/**
 * A reactor is a device that:
 * -    uses Linux epoll to allow client code to watch for events on file descriptors, and
 * -    to schedule callback functions (via runloop_post())  to be run at some point in the future
 *
 * The key feature of the device is that many file descriptors can be monitored simultaiously,
 * and many callbacks can be waiting for execution. In this regard a reactor is like
 * a lightweight task scheduler
 *
 * To watch or observe a file descriptor for events an object of type RunloopWatcher (or derived from RunloopWatcher)
 * must be created and passed to the reactor.
 *
 * The RunloopWatcher holds at least 2 pieces of information:
 * -    a function to be called when an event of interest happens
 * -    optionally a context pointer for the callback function
 * in this sense the various watchers are generalizations of a callback closure
 *
 * For convenience a number of special purposes watchers/observers have been provided.
 */
RunloopRef runloop_get_threads_reactor();
RunloopRef runloop_new(void);
void       runloop_close(RunloopRef athis);
void       runloop_init(RunloopRef athis);
void       runloop_free(RunloopRef athis);
int        runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherRef wref);
int        runloop_deregister(RunloopRef athis, int fd);
int        runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherRef wref);
int        runloop_run(RunloopRef athis, time_t timeout);
int        runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void       runloop_interthread_post(RunloopRef athis, PostableFunction cb, void* arg);
void       runloop_delete(RunloopRef athis, int fd);
void       runloop_enable_interthread_queue(RunloopRef runloop_ref);
void       runloop_verify(RunloopRef r);

RunloopRef Watcher_get_reactor(RunloopWatcherRef athis);
int        Watcher_get_fd(RunloopWatcherRef this);

/**
 * A RunloopTimer is a spcial type of RunloopWatcher that make it easy to use a Runloop
 * to implement single shot and repeating timers.
*/
RunloopTimerRef runloop_timer_new(RunloopRef runloop_ref);
void runloop_timer_init(RunloopTimerRef this, RunloopRef runloop);
void runloop_timer_free(RunloopTimerRef athis);
void runloop_timer_register(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_update(RunloopTimerRef athis, uint64_t interval_ms, bool repeating);
void runloop_timer_disarm(RunloopTimerRef athis);
void runloop_timer_rearm_old(RunloopTimerRef athis, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
void runloop_timer_rearm(RunloopTimerRef athis);
void runloop_timer_deregister(RunloopTimerRef athis);
void WTimerFd_verify(RunloopTimerRef r);
RunloopRef runloop_timer_get_reactor(RunloopTimerRef athis);
int runloop_timer_get_fd(RunloopTimerRef this);
//#define runloop_timer_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)
/**
 * Convenience intrface for timers
 */
RunloopTimerRef runloop_timer_set(RunloopRef rl, PostableFunction cb, void* ctx, uint64_t interval_ms, bool repeating);
/**
 * After the call to runloop_timer_clear the timerref is invalid and muts not be ised
 */
void runloop_timer_clear(RunloopRef rl, RunloopTimerRef timerref);

/**
 * RunloopListener is a special kind of RunloopWatcher that is intended spcifically for situations
 * where code is a server of sometype waiting for a listen() call to return signalling
 * a new connection.
 */
RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef athis);
void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef athis);
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef athis);
void runloop_listener_verify(RunloopListenerRef r);
RunloopRef runloop_listener_get_reactor(RunloopListenerRef athis);
int runloop_listener_get_fd(RunloopListenerRef this);
//#define runloop_listenerFd_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)

/**
 * RunloopStreamRef is a special type of watcher designed to watch stream style file descriptors
 * (such a sockets and pipes) to detect when such an fd is ready to perform a read operation or ready
 * to perform a write operation without BLOCKING. This type of watcher is intended for situations
 * where a single thread may be performing both read and write operations on the same fd and on
 * multiple file descriptors.
 */
RunloopStreamRef runloop_stream_new(RunloopRef runloop, int fd);
void runloop_stream_init(RunloopStreamRef athis, RunloopRef runloop, int fd);
void runloop_stream_free(RunloopStreamRef athis);
void runloop_stream_register(RunloopStreamRef athis);
void runloop_stream_deregister(RunloopStreamRef athis);
void runloop_stream_arm_both(RunloopStreamRef athis, SocketEventHandler event_handler, void* arg);
void runloop_stream_arm_read(RunloopStreamRef athis, SocketEventHandler event_handler, void* arg);
void runloop_stream_arm_write(RunloopStreamRef athis, SocketEventHandler event_handler, void* arg);
void runloop_stream_disarm_read(RunloopStreamRef athis);
void runloop_stream_disarm_write(RunloopStreamRef athis);
void runloop_stream_verify(RunloopStreamRef r);
RunloopRef runloop_stream_get_reactor(RunloopStreamRef athis);
int runloop_stream_get_fd(RunloopStreamRef this);
//#define runloop_stream_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 */
RunloopEventfdRef runloop_eventfd_new(RunloopRef runloop);
void runloop_eventfd_init(RunloopEventfdRef athis, RunloopRef runloop);
void runloop_eventfd_free(RunloopEventfdRef athis);
void runloop_eventfd_register(RunloopEventfdRef athis);
void runloop_eventfd_change_watch(RunloopEventfdRef athis, PostableFunction postable, void* arg, uint64_t watch_what);
void runloop_eventfd_arm(RunloopEventfdRef athis, PostableFunction postable, void* arg);
void runloop_eventfd_disarm(RunloopEventfdRef athis);
void runloop_eventfd_fire(RunloopEventfdRef athis);
void runloop_eventfd_clear_one_event(RunloopEventfdRef athis);
void runloop_eventfd_clear_all_events(RunloopEventfdRef athis);
void runloop_eventfd_deregister(RunloopEventfdRef athis);
void runloop_eventfd_verify(RunloopEventfdRef r);
RunloopRef runloop_eventfd_get_reactor(RunloopEventfdRef athis);
int runloop_eventfd_get_fd(RunloopEventfdRef this);
//#define runloop_eventfd_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)

/**
 * An eventfd_queue is a queue intended between threads where the act of
 * adding an element triggers an epoll eventfd to the receiving thread.
 *
 * The queue is protected by a condition variable and mutex
 */
EventfdQueueRef runloop_eventfd_queue_new();
void  runloop_eventfd_queue_free(EventfdQueueRef athis);
int   runloop_eventfd_queue_readfd(EventfdQueueRef athis);
void  runloop_eventfd_queue_add(EventfdQueueRef athis, Functor item);
Functor runloop_eventfd_queue_remove(EventfdQueueRef athis);
//RunloopRef runloop_eventfd_queue_get_reactor(EventfdQueueRef athis);
//#define runloop_eventfd_queue_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)

/**
 * A runloop_queue_watcher is the mechanism a thread uses to recieve an event whenever
 * a new entry is added to the queue by the same or a different thread.
 */
RunloopQueueWatcherRef runloop_queue_watcher_new(RunloopRef runloop, EventfdQueueRef qref);
void runloop_queue_watcher_dispose(RunloopQueueWatcherRef* athis);
void runloop_queue_watcher_register(RunloopQueueWatcherRef athis, PostableFunction postable_cb, void* postable_arg);
void runloop_queue_watcher_change_watch(RunloopQueueWatcherRef athis, QueueEventHandler cb, void* arg, uint64_t watch_what);
void runloop_queue_watcher_deregister(RunloopQueueWatcherRef athis);
void runloop_queue_watcher_verify(RunloopQueueWatcherRef r);
RunloopRef runloop_queue_watcher_get_reactor(RunloopQueueWatcherRef athis);
int runloop_queue_watcher_get_fd(RunloopQueueWatcherRef this);
//#define runloop_queue_watcher_get_reactor(p) Watcher_get_reactor((RunloopWatcherRef)p)

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
void itqueue_add(InterthreadQueueRef qref, Functor func);
Functor itqueue_remove(InterthreadQueueRef qref);

//RunloopInterthreadQueueRef runloop_interthread_queue_new(RunloopRef runloop_ref);
//void runloop_interthread_queue_init(RunloopInterthreadQueueRef this, RunloopRef runloop);
//void runloop_interthread_queue_dispose(RunloopInterthreadQueueRef this);
//void runloop_interthread_queue_add(RunloopInterthreadQueueRef this, void* item);
//void runloop_interthread_queue_drain(RunloopInterthreadQueueRef this, void(*draincb)(void*));
//void runloop_interthread_queue_register(RunloopInterthreadQueueRef this, InterthreadQueueEventHandler evhandler, void* arg, uint64_t watch_what);
//void runloop_interthreaD_queue_deregister(RunloopInterthreadQueueRef this);
//RunloopRef runloop_interthread_queue_get_reactor(RunloopInterthreadQueueRef athis);
//int runloop_interthread_queue_get_fd(RunloopInterthreadQueueRef athis);
//void runloop_interthread_queue_verify(RunloopInterthreadQueueRef this);


#endif