#ifndef c_http_xr_types_h
#define c_http_xr_types_h

typedef enum XrWatcherType {
    XR_WATCHER_SOCKET,
    XR_WATCHER_TIMER,
    XR_WATCHER_QUEUE,
} XrWatcherType;

typedef struct XrWatcher_s XrWatcher, *XrWatcherRef;
typedef struct XrTimerWatcher_s XrTimerWatcher, *XrTimerWatcherRef;
typedef struct XrSocketWatcher_s XrSocketWatcher, *XrSocketWatcherRef;
typedef struct XrQueueWatcher_s XrQueueWatcher, *XrQueueWatcherRef;
typedef struct XrRunloop_s XrRunloop, *XrRunloopRef;


#endif