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

#define XR_ASSERT(test, msg) \
do { \
    if(!test) { \
        XR_PRINTF("XR_ASSERT failed file: %s line %d msg: %s", __FILE__, __LINE__, msg ); \
        assert(test); \
    } \
} while(0)

#define XR_FATAL_ERROR(msg) \
do { \
    XR_PRINTF("Fatal error file: %s line %d msg: %s", __file__, __line__, msg ); \
    assert(false); \
} while(0)

#ifdef XR_PRINTF_ENABLE
#define XR_PRINTF(...) printf(__VA_ARGS__)
#else
#define XR_PRINTF(...)
#endif

#endif