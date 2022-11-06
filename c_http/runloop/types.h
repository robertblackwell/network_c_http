#ifndef c_http_runloop_types_h
#define c_http_runloop_types_h
#include <stdint.h>

typedef enum WatcherType {
    XR_WATCHER_SOCKET = 11,
    XR_WATCHER_TIMER = 12,
    XR_WATCHER_QUEUE = 13,
    XR_WATCHER_FDEVENT = 14,
    XR_WATCHER_LISTENER = 15,
} WatcherType;
/**
 * Forward declarations
 */
typedef struct Watcher_s Watcher, *WatcherRef;
typedef struct WTimerFd_s WTimer, *WTimerFdRef;
typedef struct WIoFd_s WSocket, *WIoFdRef;
typedef struct WQueue_s WQueue, *WQueueRef;
typedef struct WListenerFd_s WListenerFd, *WListenerFdRef;
typedef struct WEventFd_s WFdEvent, *WFdEventRef;

typedef struct Reactor_s XrReactor, *ReactorRef;

/**
 * A generic callback function - @TODO will be the signature of the only type of function that can be posted
 */
typedef void (*PostableFunction)(void* arg);
/**
 * Signature of functions that can called by the Reactor to handle file descriptor events
 */
typedef void (*WatcherCallback)(WatcherRef wref, void* arg, uint64_t events);
/**
 * Type specific event handlers - these are all the same except for the casting of the first arg to a specific type of pointer
 */
typedef void (*WatcherEventHandler)(WatcherRef wref, void* arg, uint64_t events);
typedef void (TimerEventHandler)(WTimerFdRef timer_watcher_ref, void* arg, uint64_t events);
typedef void (SocketEventHandler)(WIoFdRef socket_watcher_ref, void* arg, uint64_t events);
typedef void (FdEventHandler)(WFdEventRef fd_event_ref, void* arg, uint64_t events);
typedef void (QueueEventHandler)(WQueueRef qref, void* arg, uint64_t events);
typedef void (ListenerEventHandler)(WListenerFdRef listener_ref, void* arg, uint64_t events);

#endif