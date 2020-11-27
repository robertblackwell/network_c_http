#ifndef c_http_queue_watcher_h
#define c_http_queue_watcher_h
#include <time.h>
#include <stdint.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/evfd_queue.h>

struct XrQueueWatcher_s;
typedef struct XrQueueWatcher_s XrQueueWatcher, *XrQueueWatcherRef;
typedef uint64_t XrQueueEvent;

typedef void(XrQueueWatcherCallback(XrQueueWatcherRef watcher, void* arg, XrQueueEvent event));
typedef void(XrQueuetWatcherCaller(void* ctx));

struct XrQueueWatcher_s {
    struct XrWatcher_s;
    EvfdQueueRef            queue;
    // reactor cb and arg
    XrQueueWatcherCallback* cb;
    void*                   cb_ctx;
};

XrQueueWatcherRef Xrqw_new(XrReactorRef runloop, EvfdQueueRef qref);
void Xrqw_free(XrQueueWatcherRef this);
void Xrqw_register(XrQueueWatcherRef this, XrQueueWatcherCallback cb, void* arg,  uint64_t watch_what);
void Xrqw_change_watch(XrQueueWatcherRef this, XrQueueWatcherCallback cb, void* arg, uint64_t watch_what);

void Xrqw_deregister(XrQueueWatcherRef this);



#endif