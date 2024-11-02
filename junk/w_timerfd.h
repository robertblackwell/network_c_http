#ifndef c_http_w_timerfd_h
#define c_http_w_timerfd_h
#if 0
#include <stdint.h>
#include <stdbool.h>
#include <http_in_c/runloop/types.h>
#include <http_in_c/runloop/reactor.h>
#include <http_in_c/runloop/watcher.h>

#define TYPE WTimerFd
#define WTimerFd_TAG "XRTW"
#include <http_in_c/check_tag.h>
#undef TYPE
#define XRTW_DECLARE_TAG DECLARE_TAG(WTimerFd)
#define XR_WTIMER_CHECK_TAG(p) CHECK_TAG(WTimerFd, p)
#define XR_WTIMER_SET_TAG(p) SET_TAG(WTimerFd, p)


struct WTimerFd_s;
typedef struct WTimerFd_s WTimerFd, *WTimerFdRef;
typedef uint64_t XrTimerEvent;


struct WTimerFd_s {
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
 * @param runloop_ref       ReactorRef
 * @param cb             TimerEventHandler      an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 * @return WTimerFdRef
 */
WTimerFdRef WTimerFd_new(ReactorRef runloop_ref, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);

/**
 * Release all attached resources, deregister the timer from the Reactor and free memory.
 * @param this
 */
void WTimerFd_free(WTimerFdRef this);
/**
 * Set new values for the timer parameters
 * @param this
 * @param cb             TimerEventHandler      an event handler function
 * @param ctx            void*                  argument for the event handler
 * @param interval_ms    uint64_t               timer interval in ms
 * @param repeating      bool                   Whether repeating or not
 */
void WTimerFd_set(WTimerFdRef this, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);

void WTimerFd_update(WTimerFdRef this, uint64_t interval_ms, bool repeating);

void WTimerFd_disarm(WTimerFdRef this);
void WTimerFd_rearm_old(WTimerFdRef this, TimerEventHandler cb, void* ctx, uint64_t interval_ms, bool repeating);
void WTimerFd_rearm(WTimerFdRef this);
void WTimerFd_clear(WTimerFdRef this);

#endif

#endif