#ifndef c_http_twatcher_h
#define c_http_twatcher_h
#include <c_http/xr/runloop.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/watcher.h>

struct XrTimerWatcher_s;
typedef struct XrTimerWatcher_s XrTimerWatcher, *XrTimerWatcherRef;
typedef uint64_t XrTimerEvent;

typedef void(XrTimerWatcherCallback(XrTimerWatcherRef watcher, XrTimerEvent event));
typedef void(XrTimerWatcherCaller(void* ctx, int fd, uint64_t event));

struct XrTimerWatcher_s {
    struct XrWatcher_s;
    /**
     * XrTimerWatecher specific properties
     */
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    XrTimerWatcherCallback* cb;
};

XrTimerWatcherRef Xrtw_new(XrRunloopRef rl);
void Xrtw_free(XrTimerWatcherRef this);
void Xrtw_register(XrTimerWatcherRef this, XrTimerWatcherCallback cb, uint64_t interval);
void Xrtw_unregister(XrTimerWatcherRef this);



#endif