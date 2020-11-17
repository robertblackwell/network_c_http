#ifndef c_http_socketwatcher_h
#define c_http_socketwatcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/runloop.h>
#include <c_http/xr/watcher.h>

typedef uint64_t XrSocketEvent;

typedef void(XrSocketWatcherCallback(XrSocketWatcherRef watcher, XrSocketEvent event));
typedef void(XrSocketWatcherCaller(void* ctx));

struct XrSocketWatcher_s {
    struct XrWatcher_s;

    XrSocketWatcherCallback* cb;

};

XrSocketWatcherRef Xrsw_new(XrRunloopRef runloop);
void Xrsw_free(XrSocketWatcherRef this);
void Xrtw_watch(XrSocketWatcherRef this, uint64_t watch_what);
void Xrtw_change_watch(XrSocketWatcherRef this, uint64_t watch_what);
void Xrtw_clear(XrSocketWatcherRef this);


#endif