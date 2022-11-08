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

struct Watcher_s;
typedef struct Watcher_s Watcher, *WatcherRef;       // Base object for objects that wait for an fd event

// There are 5 types of evets that can be waited for 
// each type event-wait required a specific type of opaque object
// in order to perform such an event wait
struct WTimerFd_s;
typedef struct WTimerFd_s WTimerFd,  *WTimerFdRef;      // Wait for a timer event

struct WIoFd_s;
typedef struct WIoFd_s WIoFd, *WIoFdRef;         // Wait for an IO event on a fd

struct WQueue_s;
typedef struct WQueue_s WQueue, *WQueueRef;        // Wait for a inter thread queue event

struct WListenerFd_s;
typedef struct WListenerFd_s WListenerFd, * WListenerFdRef;   // Special event for socket listen()

struct WEventFd_s;
typedef struct WEventFd_s WEventFd, *WEventFdRef;      // Waiter for epoll event fd event

struct EvfdQueue_s;
typedef struct EvfdQueue_s EvfdQueue, * EvfdQueueRef;

typedef uint64_t EventMask, XrTimerEvent;

// A generic callback function - @TODO will be the signature of the only type of function that can be posted
typedef void (*PostableFunction)    (void* arg);
// Signature of functions that can called by the Reactor to handle file descriptor events
typedef void (*WatcherCallback)     (WatcherRef wref, void* arg, uint64_t events);
// Type specific event handlers - these are all the same except for the casting of the first arg to a specific type of pointer
typedef void (*WatcherEventHandler) (WatcherRef wref, void* arg, uint64_t events);
typedef void (*TimerEventHandler)   (WTimerFdRef timer_watcher_ref, void* arg, uint64_t events);
typedef void (*SocketEventHandler)  (WIoFdRef socket_watcher_ref, void* arg, uint64_t events);
typedef void (*FdEventHandler)      (WEventFdRef fd_event_ref, void* arg, uint64_t events);
typedef void (*QueueEventHandler)   (WQueueRef qref, void* arg, uint64_t events);
typedef void (*ListenerEventHandler)(WListenerFdRef listener_ref, void* arg, uint64_t events);


ReactorRef XrReactor_new(void);
void XrReactor_close(ReactorRef rtor_ref);
void XrReactor_free(ReactorRef rtor_ref);
int  XrReactor_register(ReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);
int  XrReactor_deregister(ReactorRef rtor_ref, int fd);
int  XrReactor_reregister(ReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);
int  XrReactor_run(ReactorRef rtor_ref, time_t timeout);
int  XrReactor_post(ReactorRef rtor_ref, PostableFunction cb, void* arg);
void XrReactor_delete(ReactorRef rtor_ref, int fd);
void XrReactor_verify(ReactorRef r);
ReactorRef Watcher_get_reactor(WatcherRef athis);
int Watcher_get_fd(WatcherRef this);
// Create a new timer event source. This function will create a new Timer object
// and register it with the provided XrReactor. In order that the timer is completely
// specified an event handler, void* arg, interval_ms and bool repeating must be provided.
WTimerFdRef WTimerFd_new(ReactorRef rtor_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void WTimerFd_free(WTimerFdRef athis);
void WTimerFd_set(WTimerFdRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void WTimerFd_update(WTimerFdRef athis, uint64_t interval_ms, bool repeating);
void WTimerFd_disarm(WTimerFdRef athis);
void WTimerFd_rearm_old(WTimerFdRef athis, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void WTimerFd_rearm(WTimerFdRef athis);
void WTimerFd_clear(WTimerFdRef athis);
void WTimerFd_verify(WTimerFdRef r);

ReactorRef WTimerFd_get_reactor(WTimerFdRef athis);
int WTimerFd_get_fd(WTimerFdRef this);
//#define WTimerFd_get_reactor(p) Watcher_get_reactor((WatcherRef)p)


EvfdQueueRef Evfdq_new();
void  Evfdq_free(EvfdQueueRef athis);
int   Evfdq_readfd(EvfdQueueRef athis);
void  Evfdq_add(EvfdQueueRef athis, void* item);
void* Evfdq_remove(EvfdQueueRef athis);
//ReactorRef Evfdq_get_reactor(EvfdQueueRef athis);
//#define Evfdq_get_reactor(p) Watcher_get_reactor((WatcherRef)p)

WListenerFdRef WListenerFd_new(ReactorRef runloop, int fd);
void WListenerFd_free(WListenerFdRef athis);
void WListenerFd_register(WListenerFdRef athis, ListenerEventHandler event_handler, void* arg);
void WListenerFd_deregister(WListenerFdRef athis);
void WListenerFd_arm(WListenerFdRef athis, ListenerEventHandler fd_event_handler, void* arg);
void XrLIstener_disarm(WListenerFdRef athis);
void WListenerFd_verify(WListenerFdRef r);
ReactorRef WListener_get_reactor(WListenerFdRef athis);
int WListenerFd_get_fd(WListenerFdRef this);
//#define WListenerFd_get_reactor(p) Watcher_get_reactor((WatcherRef)p)

WIoFdRef WIoFd_new(ReactorRef runloop, int fd);
void WIoFd_free(WIoFdRef athis);
void WIoFd_register(WIoFdRef athis);
void WIoFd_deregister(WIoFdRef athis);
void WIoFd_arm_read(WIoFdRef athis, SocketEventHandler event_handler, void* arg);
void WIoFd_arm_write(WIoFdRef athis, SocketEventHandler event_handler, void* arg);
void WIoFd_disarm_read(WIoFdRef athis);
void WIoFd_disarm_write(WIoFdRef athis);
void WIoFd_verify(WIoFdRef r);
ReactorRef WIoFd_get_reactor(WIoFdRef athis);
int WIoFd_get_fd(WIoFdRef this);
//#define WIoFd_get_reactor(p) Watcher_get_reactor((WatcherRef)p)

WQueueRef WQueue_new(ReactorRef runloop, EvfdQueueRef qref);
void WQueue_dispose(WQueueRef athis);
void WQueue_register(WQueueRef athis, QueueEventHandler cb, void* arg,  uint64_t watch_what);
void WQueue_change_watch(WQueueRef athis, QueueEventHandler cb, void* arg, uint64_t watch_what);
void WQueue_deregister(WQueueRef athis);
void WQueue_verify(WQueueRef r);
ReactorRef WQueue_get_reactor(WQueueRef athis);
int WQueue_get_fd(WQueueRef this);
//#define WQueue_get_reactor(p) Watcher_get_reactor((WatcherRef)p)

WEventFdRef WEventFd_new(ReactorRef runloop);
void WEventFd_free(WEventFdRef athis);
void WEventFd_register(WEventFdRef athis);
void WEventFd_change_watch(WEventFdRef athis, FdEventHandler evhandler, void* arg, uint64_t watch_what);
void WEventFd_arm(WEventFdRef athis,  FdEventHandler evhandler, void* arg);
void WEventFd_disarm(WEventFdRef athis);
void WEventFd_fire(WEventFdRef athis);
void WEventFd_deregister(WEventFdRef athis);
void WEventFd_verify(WEventFdRef r);
ReactorRef WEventFd_get_reactor(WEventFdRef athis);
int WEventFd_get_fd(WEventFdRef this);
//#define WEventFd_get_reactor(p) Watcher_get_reactor((WatcherRef)p)

#endif