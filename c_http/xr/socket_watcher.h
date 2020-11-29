#ifndef c_http_socketwatcher_h
#define c_http_socketwatcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>

typedef uint64_t XrSocketEvent;

typedef void(XrSocketWatcherCallback(XrWatcherRef watch, void* arg, uint64_t event));
typedef void(XrSocketWatcherCaller(void* ctx));

struct XrSocketWatcher_s {
    struct XrWatcher_s;

    XrSocketWatcherCallback* cb;
    void*                    cb_ctx;

};

XrSocketWatcherRef Xrsw_new(XrReactorRef runloop, int fd);
void Xrsw_free(XrSocketWatcherRef this);
void Xrsw_register(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg,  uint64_t watch_what);
void Xrsw_change_watch(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what);

void Xrsw_deregister(XrSocketWatcherRef this);


#endif