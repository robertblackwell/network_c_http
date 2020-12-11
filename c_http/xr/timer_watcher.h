#ifndef c_http_twatcher_h
#define c_http_twatcher_h
#include <c_http/xr/reactor.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/watcher.h>

#define TYPE XrTimer
#define XrTimer_TAG "XRTW"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_FDEV_DECLARE_TAG DECLARE_TAG(XrTimer)
#define XRTW_CHECK_TAG(p) CHECK_TAG(XrTimer, p)
#define XRTW_SET_TAG(p) SET_TAG(XrTimer, p)


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
/**
 * Create a new timer event source. This function will create a new Timer object
 * and register it with the provided XrReactor. In order that the timer is completely
 * specified a callback, void* arg, interval_ms and bool repeating must be provided.
 * @param rtor_ref       XrReactorRef
 * @param cb             XrTimerWatcherCallback an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 * @return XrTimerWatcherRef
 */
XrTimerWatcherRef Xrtw_new(XrReactorRef rtor_ref, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating);

/**
 * Release all attached resources, deregister the timer from the Reactor and free memory.
 * @param this
 */
void Xrtw_free(XrTimerWatcherRef this);
/**
 * Set new values for the timer parameters
 * @param this
 * @param cb             XrTimerWatcherCallback an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 */
void Xrtw_set(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating);

void Xrtw_update(XrTimerWatcherRef this, uint64_t interval_ms, bool repeating);

void Xrtw_disarm(XrTimerWatcherRef this);
void Xrtw_rearm_old(XrTimerWatcherRef this, XrTimerWatcherCallback cb, void* ctx, uint64_t interval_ms, bool repeating);
void Xrtw_rearm(XrTimerWatcherRef this);
void Xrtw_clear(XrTimerWatcherRef this);



#endif