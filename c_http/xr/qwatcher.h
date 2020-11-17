#ifndef c_http_queue_watcher_h
#define c_http_queue_watcher_h
#include <time.h>
#include <stdint.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/runloop.h>

struct XrQueueWatcher_s;
typedef struct XrQueueWatcher_s XrQueueWatcher, *XrQueueWatcherRef;
typedef uint64_t XrQueueEvent;

typedef void(XrQueueWatcherCallback(XrQueueWatcherRef watcher, void* arg, XrQueueEvent event));
typedef void(XrQueuetWatcherCaller(void* ctx));

struct XrQueueWatcher_s {
    struct XrWatcher_s;
    void*                   queue;
    XrQueueWatcherCallback* cb;
};

XrQueueWatcherRef Xrqw_new(XrRunloopRef runloop);
void Xrqw_free(XrQueueWatcherRef this);


#endif