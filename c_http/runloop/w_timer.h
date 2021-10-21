#ifndef c_http_twatcher_h
#define c_http_twatcher_h
#include <c_http/runloop/reactor.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/runloop/watcher.h>

#define TYPE WTimer
#define WTimer_TAG "XRTW"
#include <c_http/check_tag.h>
#undef TYPE
#define XRTW_DECLARE_TAG DECLARE_TAG(WTimer)
#define XR_WTIMER_CHECK_TAG(p) CHECK_TAG(WTimer, p)
#define XR_WTIMER_SET_TAG(p) SET_TAG(WTimer, p)


struct WTimer_s;
typedef struct WTimer_s WTimer, *WTimerRef;
typedef uint64_t XrTimerEvent;

typedef void(WTimerCaller(void* ctx, int fd, uint64_t event));

struct WTimer_s {
    struct Watcher_s;
    /**
     * XrTimerWatecher specific properties
     */
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    TimerEventHandler*      timer_handler;
    void*                   timer_handler_arg;
};
/**
 * Create a new timer event source. This function will create a new Timer object
 * and register it with the provided XrReactor. In order that the timer is completely
 * specified an event handler, void* arg, interval_ms and bool repeating must be provided.
 * 
 * @param rtor_ref       XrReactorRef
 * @param cb             TimerEventHandler      an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 * @return WTimerRef
 */
WTimerRef WTimer_new(XrReactorRef rtor_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);

/**
 * Release all attached resources, deregister the timer from the Reactor and free memory.
 * @param this
 */
void WTimer_free(WTimerRef this);
/**
 * Set new values for the timer parameters
 * @param this
 * @param cb             TimerEventHandler      an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 */
void WTimer_set(WTimerRef this, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);

void WTimer_update(WTimerRef this, uint64_t interval_ms, bool repeating);

void WTimer_disarm(WTimerRef this);
void WTimer_rearm_old(WTimerRef this, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void WTimer_rearm(WTimerRef this);
void WTimer_clear(WTimerRef this);



#endif