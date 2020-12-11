#ifndef c_http_fdevent_h
#define c_http_fdevent_h
#include <time.h>
#include <stdint.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/reactor.h>

#define TYPE XrFdEv
#define XrFdEv_TAG "XRFDEV"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_FDEV_DECLARE_TAG DECLARE_TAG(XrFdEv)
#define XR_FDEV_CHECK_TAG(p) CHECK_TAG(XrFdEv, p)
#define XR_FDEV_SET_TAG(p) SET_TAG(XrFdEv, p)

/**
 * An XrFdEvent is a generic type of event that can be waited on and fired. It is intended to provide
 * a generic async signalling mechanism between threads within a process.
 *
 * Underneath it makes use of the Linux eventfd() mechanism.
 *
 * This "type" is a derived type with XrWatcher as the base.
 *
 */
struct XrFdEvent_s;
typedef struct XrFdEvent_s XrFdEvent, *XrFdEventRef;
typedef uint64_t XrFdEventMask;

typedef void(XrFdEventCallback(XrFdEventRef event, void* arg, XrFdEventMask evmask));
typedef void(XrFdEventCaller(void* ctx));

struct XrFdEvent_s {
    struct XrWatcher_s;

    XrFdEventCallback* cb;
    void*            cb_ctx;
    int              write_fd;
};

XrFdEventRef XrFdEvent_new(XrReactorRef runloop);
void XrFdEvent_free(XrFdEventRef this);
void XrFdEvent_register(XrFdEventRef this);
void Xrqw_change_watch(XrFdEventRef this, XrFdEventCallback cb, void* arg, uint64_t watch_what);

void XrFdEvent_arm(XrFdEventRef this,  FdEventHandler evhandler, void* arg);
void XeFdEvent_disarm(XrFdEventRef this);
void XeFdEvent_fire(XrFdEventRef this);
void XrFdEvent_deregister(XrFdEventRef this);
void XrFdEvent_fire(XrFdEventRef this);




#endif