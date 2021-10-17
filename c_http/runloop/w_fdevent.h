#ifndef c_http_fdevent_h
#define c_http_fdevent_h
#include <time.h>
#include <stdint.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/reactor.h>

#define TYPE WFdEvent
#define WFdEvent_TAG "XRFDEV"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_FDEV_DECLARE_TAG DECLARE_TAG(WFdEvent)
#define XR_FDEV_CHECK_TAG(p) CHECK_TAG(WFdEvent, p)
#define XR_FDEV_SET_TAG(p) SET_TAG(WFdEvent, p)

/**
 * An WFdEvent is a generic type of event that can be waited on and fired. It is intended to provide
 * a generic async signalling mechanism between threads within a process.
 *
 * Underneath it makes use of the Linux eventfd() mechanism.
 *
 * This "type" is a derived type with Watcher as the base.
 *
 */
struct WFdEvent_s;
typedef struct WFdEvent_s WFdEvent, *WFdEventRef;
typedef uint64_t WFdEventMask;

typedef void(WFdEventCaller(void* ctx));

struct WFdEvent_s {
    struct Watcher_s;

    FdEventHandler*     fd_event_handler;
    void*               fd_event_handler_arg;
    int                 write_fd;
};

WFdEventRef WFdEvent_new(XrReactorRef runloop);
void WFdEvent_free(WFdEventRef this);
void WFdEvent_register(WFdEventRef this);
void WQueue_change_watch(WFdEventRef this, FdEventHandler evhandler, void* arg, uint64_t watch_what);

void WFdEvent_arm(WFdEventRef this,  FdEventHandler evhandler, void* arg);
void WFdEvent_disarm(WFdEventRef this);
void WFdEvent_fire(WFdEventRef this);
void WFdEvent_deregister(WFdEventRef this);
void WFdEvent_fire(WFdEventRef this);




#endif