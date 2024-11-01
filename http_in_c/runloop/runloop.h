#ifndef C_HTTP_RUNLOOP_H
#define C_HTTP_RUNLOOP_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Opaque objects
 */
struct Reactor_s;
typedef struct Reactor_s Reactor, *ReactorRef;

struct RtorWatcher_s;
typedef struct RtorWatcher_s RtorWatcher, *RtorWatcherRef;       // Base object for objects that wait for an fd event

// There are 5 types of evets that can be waited for 
// each type event-wait required a specific type of opaque object
// in order to perform such an event wait
struct RtorTimer_s;
typedef struct RtorTimer_s RtorTimer,  *RtorTimerRef;      // Wait for a timer event

struct RtorStream_s;
typedef struct RtorStream_s RtorStream, *RtorStreamRef;         // Wait for an IO event on a fd

struct RtorWQueue_s;
typedef struct RtorWQueue_s RtorWQueue, *RtorWQueueRef;        // Wait for a inter thread queue event

struct RtorListener_s;
typedef struct RtorListener_s RtorListener, * RtorListenerRef;   // Special event for socket listen()

struct RtorEventfd_s;
typedef struct RtorEventfd_s RtorEventfd, *RtorEventfdRef;      // Waiter for epoll event fd event

struct EvfdQueue_s;
typedef struct EvfdQueue_s EvfdQueue, * EvfdQueueRef;

struct RtorInterthreadQueue_s;
typedef struct RtorInterthreadQueue_s RtorInterthreadQueue, *RtorInterthreadQueueRef;        // Wait for a inter thread queue event

struct Functor_s;
typedef struct Functor_s Functor;

typedef uint64_t EventMask, XrTimerEvent;

// A generic callback function
typedef void (*PostableFunction)    (ReactorRef rtor_ref, void* arg);
// Signature of functions that can called by the Reactor to handle file descriptor events
typedef void (*WatcherCallback)     (RtorWatcherRef wref, uint64_t events);
// Type specific event handlers - these are all the same except for the casting of the first arg to a specific type of pointer
typedef void (*WatcherEventHandler) (RtorWatcherRef wref, uint64_t events);
typedef void (*TimerEventHandler)   (RtorTimerRef timer_watcher_ref, uint64_t events);
typedef void (*SocketEventHandler)  (RtorStreamRef socket_watcher_ref, uint64_t events);
typedef void (*FdEventHandler)      (RtorEventfdRef fd_event_ref, uint64_t events);
typedef void (*QueueEventHandler)   (RtorWQueueRef qref, uint64_t events);
typedef void (*InterthreadQueueEventHandler)   (RtorInterthreadQueueRef qref);
typedef void (*ListenerEventHandler)(RtorListenerRef listener_ref, uint64_t events);

/**
 * A reactor is a device that:
 * -    uses Linux epoll to allow client code to watch for events on file descriptors, and
 * -    to schedule callback functions (via rtor_reactor_post())  to be run at some point in the future
 *
 * The key feature of the device is that many file descriptors can be monitored simultaiously,
 * and many callbacks can be waiting for execution. In this regard a reactor is like
 * a lightweight task scheduler
 *
 * To watch or observe a file descriptor for events an object of type RtorWatcher (or derived from RtorWatcher)
 * must be created and passed to the reactor.
 *
 * The RtorWatcher holds at least 2 pieces of information:
 * -    a function to be called when an event of interest happens
 * -    optionally a context pointer for the callback function
 * in this sense the various watchers are generalizations of a callback closure
 *
 * For convenience a number of special purposes watchers/observers have been provided.
 */
ReactorRef rtor_reactor_get_threads_reactor();
ReactorRef rtor_reactor_new(void);
void       rtor_reactor_close(ReactorRef athis);
void       rtor_reactor_init(ReactorRef athis);
void       rtor_reactor_free(ReactorRef athis);
int        rtor_reactor_register(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref);
int        rtor_reactor_deregister(ReactorRef athis, int fd);
int        rtor_reactor_reregister(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref);
int        rtor_reactor_run(ReactorRef athis, time_t timeout);
int        rtor_reactor_post(ReactorRef athis, PostableFunction cb, void* arg);
void       rtor_reactor_interthread_post(ReactorRef athis, PostableFunction cb, void* arg);
void       rtor_reactor_delete(ReactorRef athis, int fd);
void       rtor_reactor_enable_interthread_queue(ReactorRef rtor_ref);
void       XrReactor_verify(ReactorRef r);

ReactorRef Watcher_get_reactor(RtorWatcherRef athis);
int        Watcher_get_fd(RtorWatcherRef this);

/**
 * A RtorTimer is a spcial type of RtorWatcher that make it easy to use a Reactor
 * to implement single shot and repeating timers.
*/
RtorTimerRef rtor_timer_new(ReactorRef rtor_ref);
void rtor_timer_init(RtorTimerRef this, ReactorRef runloop);
void rtor_timer_free(RtorTimerRef athis);
void rtor_timer_register(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_update(RtorTimerRef athis, uint64_t interval_ms, bool repeating);
void rtor_timer_disarm(RtorTimerRef athis);
void rtor_timer_rearm_old(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_rearm(RtorTimerRef athis);
void rtor_timer_deregister(RtorTimerRef athis);
void WTimerFd_verify(RtorTimerRef r);
ReactorRef rtor_timer_get_reactor(RtorTimerRef athis);
int rtor_timer_get_fd(RtorTimerRef this);
//#define rtor_timer_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

/**
 * RtorListener is a special kind of RtorWatcher that is intended spcifically for situations
 * where code is a server of sometype waiting for a listen() call to return signalling
 * a new connection.
 */
RtorListenerRef rtor_listener_new(ReactorRef runloop, int fd);
void rtor_listener_init(RtorListenerRef athis, ReactorRef runloop, int fd);
void rtor_listener_free(RtorListenerRef athis);
void rtor_listener_register(RtorListenerRef athis, ListenerEventHandler event_handler, void* arg);
void rtor_listener_deregister(RtorListenerRef athis);
void rtor_listener_arm(RtorListenerRef athis, ListenerEventHandler fd_event_handler, void* arg);
void rtor_lIstener_disarm(RtorListenerRef athis);
void rtor_listener_verify(RtorListenerRef r);
ReactorRef rtor_listener_get_reactor(RtorListenerRef athis);
int rtor_listener_get_fd(RtorListenerRef this);
//#define rtor_listenerFd_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

/**
 * RtorRdrWrtrWatcher is a special type of watcher designed to watch stream style file descriptors
 * (such a sockets and pipes) to detect when such an fd is ready to perform a read operation or ready
 * to perform a write operation without BLOCKING. This type of watcher is intended for situations
 * where a single thread may be performing both read and write operations on the same fd and on
 * multiple file descriptors.
 */
RtorStreamRef rtor_stream_new(ReactorRef runloop, int fd);
void rtor_stream_init(RtorStreamRef athis, ReactorRef runloop, int fd);
void rtor_stream_free(RtorStreamRef athis);
void rtor_stream_register(RtorStreamRef athis);
void rtor_stream_deregister(RtorStreamRef athis);
void rtor_stream_arm_both(RtorStreamRef athis, SocketEventHandler event_handler, void* arg);
void rtor_stream_arm_read(RtorStreamRef athis, SocketEventHandler event_handler, void* arg);
void rtor_stream_arm_write(RtorStreamRef athis, SocketEventHandler event_handler, void* arg);
void rtor_stream_disarm_read(RtorStreamRef athis);
void rtor_stream_disarm_write(RtorStreamRef athis);
void rtor_stream_verify(RtorStreamRef r);
ReactorRef rtor_stream_get_reactor(RtorStreamRef athis);
int rtor_stream_get_fd(RtorStreamRef this);
//#define rtor_stream_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 */
RtorEventfdRef rtor_eventfd_new(ReactorRef runloop);
void rtor_eventfd_init(RtorEventfdRef athis, ReactorRef runloop);
void rtor_eventfd_free(RtorEventfdRef athis);
void rtor_eventfd_register(RtorEventfdRef athis);
void rtor_eventfd_change_watch(RtorEventfdRef athis, FdEventHandler evhandler, void* arg, uint64_t watch_what);
void rtor_eventfd_arm(RtorEventfdRef athis, FdEventHandler evhandler, void* arg);
void rtor_eventfd_disarm(RtorEventfdRef athis);
void rtor_eventfd_fire(RtorEventfdRef athis);
void rtor_eventfd_clear_one_event(RtorEventfdRef athis);
void rtor_eventfd_clear_all_events(RtorEventfdRef athis);
void rtor_eventfd_deregister(RtorEventfdRef athis);
void rtor_eventfd_verify(RtorEventfdRef r);
ReactorRef rtor_eventfd_get_reactor(RtorEventfdRef athis);
int rtor_eventfd_get_fd(RtorEventfdRef this);
//#define rtor_eventfd_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)


EvfdQueueRef Evfdq_new();
void  Evfdq_free(EvfdQueueRef athis);
int   Evfdq_readfd(EvfdQueueRef athis);
void  Evfdq_add(EvfdQueueRef athis, Functor item);
Functor Evfdq_remove(EvfdQueueRef athis);
//ReactorRef Evfdq_get_reactor(EvfdQueueRef athis);
//#define Evfdq_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)


RtorWQueueRef rtor_wqueue_new(ReactorRef runloop, EvfdQueueRef qref);
void rtor_wqueue_dispose(RtorWQueueRef* athis);
void rtor_wqueue_register(RtorWQueueRef athis, QueueEventHandler cb, void* arg);
void rtor_wqueue_change_watch(RtorWQueueRef athis, QueueEventHandler cb, void* arg, uint64_t watch_what);
void rtor_wqueue_deregister(RtorWQueueRef athis);
void rtor_wqueue_verify(RtorWQueueRef r);
ReactorRef rtor_wqueue_get_reactor(RtorWQueueRef athis);
int rtor_wqueue_get_fd(RtorWQueueRef this);
//#define rtor_wqueue_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

RtorInterthreadQueueRef rtor_interthread_queue_new(ReactorRef rtor_ref);
void rtor_interthread_queue_init(RtorInterthreadQueueRef this, ReactorRef runloop);
void rtor_interthread_queue_dispose(RtorInterthreadQueueRef this);
void rtor_interthread_queue_add(RtorInterthreadQueueRef this, void* item);
void rtor_interthread_queue_drain(RtorInterthreadQueueRef this, void(*draincb)(void*));
void rtor_interthread_queue_register(RtorInterthreadQueueRef this, InterthreadQueueEventHandler evhandler, void* arg, uint64_t watch_what);
void rtor_interthreaD_queue_deregister(RtorInterthreadQueueRef this);
ReactorRef rtor_interthread_queue_get_reactor(RtorInterthreadQueueRef athis);
int rtor_interthread_queue_get_fd(RtorInterthreadQueueRef athis);
void rtor_interthread_queue_verify(RtorInterthreadQueueRef this);


#endif