#ifndef c_http_xr_types_h
#define c_http_xr_types_h
#include <c_http/list.h>
#include <c_http/message.h>
/**
 * This is the signature of a handler function. *The address) of such a function must
 * be provided to Server_new() in order that the server and its worker threads
 * can call on this function to handle requests.
 *
 * The handler function is called once for each request and is passed the request message in its
 * entirety together with a Writer instance that provides functions to write the response.
 *
 * The handler function is solely responsible for constructing and sending the response.
 */
typedef int(*XrHandlerFunction)(MessageRef request, void* wrttr);


typedef enum XrWatcherType {
    XR_WATCHER_SOCKET = 11,
    XR_WATCHER_TIMER = 12,
    XR_WATCHER_QUEUE = 13,
} XrWatcherType;
/**
 * Forward declarations
 */
typedef struct XrWatcher_s XrWatcher, *XrWatcherRef;
typedef struct XrTimerWatcher_s XrTimerWatcher, *XrTimerWatcherRef;
typedef struct XrSocketWatcher_s XrSocketWatcher, *XrSocketWatcherRef;
typedef struct XrQueueWatcher_s XrQueueWatcher, *XrQueueWatcherRef;
typedef struct XrReactor_s XrReactor, *XrReactorRef;
typedef struct XrWorker_s XrWorker, *XrWorkerRef;
typedef struct XrServer_s XrServer, *XrServerRef;
typedef struct XrConn_s XrConn, *XrConnRef;
typedef struct XrHandler_s XrHandler, *XrHandlerRef;

typedef ListRef XrConnListRef;
typedef ListIter XrConnListIter;
typedef void (*WatcherCallback)(XrWatcherRef wref, void* arg, uint64_t events);

typedef void (XrConnReadCallback)(XrConnRef conn, void* arg, int bytes_read, int status);
typedef void (*XrConnReadMsgCallback)(XrConnRef conn, void* arg, int status);
typedef void (*XrConnWriteCallback)(XrConnRef conn, void* arg, int status);

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

#define XR_PRINTF_ENABLE
#ifdef XR_PRINTF_ENABLE
#define XR_PRINTF(...) printf(__VA_ARGS__)
#else
#define XR_PRINTF(...)
#endif

#define XRSW_TYPE_CHECK(w) assert(w->type == XR_WATCHER_SOCKET);
#define XRTW_TYPE_CHECK(w) assert(w->type == XR_WATCHER_TIMER);
#define XRQW_TYPE_CHECK(w) assert(w->type == XR_WATCHER_QUEUE);

#endif