#ifndef c_http_runloop_h
#define c_http_runloop_h

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

struct RtorRdrWrtr_s;
typedef struct RtorRdrWrtr_s RtorRdrWrtr, *RtorRdrWrtrRef;         // Wait for an IO event on a fd

struct WQueue_s;
typedef struct WQueue_s WQueue, *WQueueRef;        // Wait for a inter thread queue event

struct RtorListener_s;
typedef struct RtorListener_s RtorListener, * RtorListenerRef;   // Special event for socket listen()

struct RtorEventfd_s;
typedef struct RtorEventfd_s RtorEventfd, *RtorEventfdRef;      // Waiter for epoll event fd event

struct EvfdQueue_s;
typedef struct EvfdQueue_s EvfdQueue, * EvfdQueueRef;

typedef uint64_t EventMask, XrTimerEvent;

// A generic callback function - @TODO will be the signature of the only type of function that can be posted
typedef void (*PostableFunction)    (void* arg);
// Signature of functions that can called by the Reactor to handle file descriptor events
typedef void (*WatcherCallback)     (RtorWatcherRef wref, void* arg, uint64_t events);
// Type specific event handlers - these are all the same except for the casting of the first arg to a specific type of pointer
typedef void (*WatcherEventHandler) (RtorWatcherRef wref, void* arg, uint64_t events);
typedef void (*TimerEventHandler)   (RtorTimerRef timer_watcher_ref, void* arg, uint64_t events);
typedef void (*SocketEventHandler)  (RtorRdrWrtrRef socket_watcher_ref, void* arg, uint64_t events);
typedef void (*FdEventHandler)      (RtorEventfdRef fd_event_ref, void* arg, uint64_t events);
typedef void (*QueueEventHandler)   (WQueueRef qref, void* arg, uint64_t events);
typedef void (*ListenerEventHandler)(RtorListenerRef listener_ref, void* arg, uint64_t events);

/**
 * A reactor is a device that uses Linux epoll to allow client code to watch for events on file descriptors.
 * The key significance of the device is that many file descriptors can be monitored simultaiously.
 *
 * To watch or observe a file descriptor for events an object of type RtorWatcher (or derived from RtorWatcher)
 * and passed to the reactor.
 *
 * The RtorWatcher holds at least 2 pieces of information:
 * -    a function to be called when an event of interest happens
 * -    optionally a context pointer for the callback function
 *
 * in this sense the various watchers are generalizations of a callback closure
 *
 * For convenience a number of special purposes watchers/observers have been provided.
 */
ReactorRef rtor_new(void);
void rtor_close(ReactorRef athis);
void rtor_init(ReactorRef athis);
void rtor_free(ReactorRef athis);
int  rtor_register(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref);
int  rtor_deregister(ReactorRef athis, int fd);
int  rtor_reregister(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref);
int  rtor_run(ReactorRef athis, time_t timeout);
int  rtor_post(ReactorRef athis, PostableFunction cb, void* arg);
int  rtor_interthread_post(ReactorRef athis, PostableFunction cb, void* arg);
void rtor_delete(ReactorRef athis, int fd);
void XrReactor_verify(ReactorRef r);
ReactorRef Watcher_get_reactor(RtorWatcherRef athis);
int Watcher_get_fd(RtorWatcherRef this);

/**
 * A RtorTimer is a spcial type of RtorWatcher that make it easy to use a Reactor
 * to implement single shot and repeating timers.
*/
RtorTimerRef rtor_timer_new(ReactorRef rtor_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_init(RtorTimerRef athis, ReactorRef rtor_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_free(RtorTimerRef athis);
void rtor_timer_set(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_update(RtorTimerRef athis, uint64_t interval_ms, bool repeating);
void rtor_timer_disarm(RtorTimerRef athis);
void rtor_timer_rearm_old(RtorTimerRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void rtor_timer_rearm(RtorTimerRef athis);
void rtor_timer_clear(RtorTimerRef athis);
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
void rtor_Listener_init(RtorListenerRef athis, ReactorRef runloop, int fd);
void rtor_Listener_free(RtorListenerRef athis);
void rtor_listener_register(RtorListenerRef athis, ListenerEventHandler event_handler, void* arg);
void rtor_listener_deregister(RtorListenerRef athis);
void rtor_listener_arm(RtorListenerRef athis, ListenerEventHandler fd_event_handler, void* arg);
void rtor_lIstener_disarm(RtorListenerRef athis);
void rtor_listener_verify(RtorListenerRef r);
ReactorRef rtor_listener_get_reactor(RtorListenerRef athis);
int rtor_listener_get_fd(RtorListenerRef this);
//#define WListenerFd_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

/**
 * RtorRdrWrtrWatcher is a special type of watcher designed to watch stream style file descriptors
 * (such a sockets and pipes) to detect when such an fd is ready to perform a read operation or ready
 * to perform a write operation without BLOCKING. This type of watcher is intended for situations
 * where a single thread may be performing both read and write operations on the same fd and on
 * multiple file descriptors.
 */
RtorRdrWrtrRef rtor_rdrwrtr_new(ReactorRef runloop, int fd);
void rtor_rdrwrtr_init(RtorRdrWrtrRef athis, ReactorRef runloop, int fd);
void rtor_rdrwrtr_free(RtorRdrWrtrRef athis);
void rtor_rdrwrtr_register(RtorRdrWrtrRef athis);
void rtor_rdrwrtr_deregister(RtorRdrWrtrRef athis);
void rtor_rdrwrtr_arm_read(RtorRdrWrtrRef athis, SocketEventHandler event_handler, void* arg);
void rtor_rdrwrtr_arm_write(RtorRdrWrtrRef athis, SocketEventHandler event_handler, void* arg);
void rtor_rdrwrtr_disarm_read(RtorRdrWrtrRef athis);
void rtor_rdrwrtr_disarm_write(RtorRdrWrtrRef athis);
void rtor_rdrwrtr_verify(RtorRdrWrtrRef r);
ReactorRef rtor_rdrwrtr_get_reactor(RtorRdrWrtrRef athis);
int rtor_rdrwrtr_get_fd(RtorRdrWrtrRef this);
//#define rtor_rdrwrtr_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)

/**
 * epoll provides a facility to create a file descriptor that is not attached to any file/pipe/device
 * and to "fire" events on that file descriptor that can be waited for using the epoll call.
 * This facility provides a mechanism to create and wait on arbitary event sources.
 */
RtorEventfdRef rtor_eventfd(ReactorRef runloop);
void rtor_eventfd_init(RtorEventfdRef athis);
void rtor_eventfd_free(RtorEventfdRef athis);
void rtor_eventfd_register(RtorEventfdRef athis);
void rtor_eventfd_change_watch(RtorEventfdRef athis, FdEventHandler evhandler, void* arg, uint64_t watch_what);
void rtor_eventfd_arm(RtorEventfdRef athis, FdEventHandler evhandler, void* arg);
void rtor_eventfd_disarm(RtorEventfdRef athis);
void rtor_eventfd_fire(RtorEventfdRef athis);
void rtor_eventfd_deregister(RtorEventfdRef athis);
void rtor_eventfd_verify(RtorEventfdRef r);
ReactorRef rtor_eventfd_get_reactor(RtorEventfdRef athis);
int rtor_eventfd_get_fd(RtorEventfdRef this);
//#define rtor_eventfd_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)


EvfdQueueRef Evfdq_new();
void  Evfdq_free(EvfdQueueRef athis);
int   Evfdq_readfd(EvfdQueueRef athis);
void  Evfdq_add(EvfdQueueRef athis, void* item);
void* Evfdq_remove(EvfdQueueRef athis);
//ReactorRef Evfdq_get_reactor(EvfdQueueRef athis);
//#define Evfdq_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)


WQueueRef WQueue_new(ReactorRef runloop, EvfdQueueRef qref);
void WQueue_dispose(WQueueRef athis);
void WQueue_register(WQueueRef athis, QueueEventHandler cb, void* arg,  uint64_t watch_what);
void WQueue_change_watch(WQueueRef athis, QueueEventHandler cb, void* arg, uint64_t watch_what);
void WQueue_deregister(WQueueRef athis);
void WQueue_verify(WQueueRef r);
ReactorRef WQueue_get_reactor(WQueueRef athis);
int WQueue_get_fd(WQueueRef this);
//#define WQueue_get_reactor(p) Watcher_get_reactor((RtorWatcherRef)p)


#endif