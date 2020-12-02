#ifndef c_http_twatcher_h
#define c_http_twatcher_h
#include <c_http/xr/reactor.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/watcher.h>

struct XrTimerWatcher_s;
typedef struct XrTimerWatcher_s XrTimerWatcher, *XrTimerWatcherRef;
typedef uint64_t XrTimerEvent;

typedef void(XrTimerWatcherCallback(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event));
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
    void*                   cb_ctx;
};

XrTimerWatcherRef Xrtw_new(XrReactorRef rtor_ref);
void Xrtw_free(XrTimerWatcherRef this);
void Xrtw_set(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating);
void Xrtw_update(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating);
void Xrtw_disarm(XrTimerWatcherRef this);
void Xrtw_rearm(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating);
void Xrtw_rearm_2(XrTimerWatcherRef this);
void Xrtw_clear(XrTimerWatcherRef this);



#endif