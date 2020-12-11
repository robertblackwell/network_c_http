#ifndef c_http_listener_watcher_h
#define c_http_listener_watcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>

#define TYPE XrListener
#define XrListener_TAG "XRLST"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_LIST_DECLARE_TAG DECLARE_TAG(XrListener)
#define XR_LIST_CHECK_TAG(p) CHECK_TAG(XrListener, p)
#define XR_LIST_SET_TAG(p) SET_TAG(XrListener, p)

#ifdef NOWAY
#define XR_SOCKW_TAG "XRWS"
#define XR_SOCKW_TAG_LENGTH 8
#define XR_SOCKW_DECLARE_TAG char tag[XR_SOCKW_TAG_LENGTH]
#define XR_SOCKW_CHECK_TAG(p) \
do { \
    assert(strcmp((p)->tag, XR_SOCKW_TAG) == 0); \
} while(0);

#define XR_SOCKW_SET_TAG(p) \
do { \
    sprintf((p)->tag, "%s", XR_SOCKW_TAG); \
} while(0);
#endif

typedef uint64_t XrSocketEvent;

typedef void(XrListenerCaller(void* ctx));

struct XrListener_s {
    struct XrWatcher_s;
    ListenerEventHandler*      listen_evhandler;
    void*                    listen_arg;


};

XrListenerRef XrListener_new(XrReactorRef runloop, int fd);
void XrListener_free(XrListenerRef this);
void XrListener_register(XrListenerRef this, ListenerEventHandler event_handler, void* arg);
void XrListener_deregister(XrListenerRef this);

/**
 * Enable reception of fd listener events and set the event handler
 * @param this            XrListenerRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param fd_event_handler
 */
void XrListener_arm(XrListenerRef this, ListenerEventHandler fd_event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void XrLIstener_disarm(XrListenerRef this);

#endif