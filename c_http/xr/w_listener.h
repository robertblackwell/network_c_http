#ifndef c_http_listener_watcher_h
#define c_http_listener_watcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>

#define TYPE WListener
#define WListener_TAG "XRLST"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_LIST_DECLARE_TAG DECLARE_TAG(WListener)
#define XR_LIST_CHECK_TAG(p) CHECK_TAG(WListener, p)
#define XR_LIST_SET_TAG(p) SET_TAG(WListener, p)


typedef uint64_t XrSocketEvent;

typedef void(XrListenerCaller(void* ctx));

struct XrListener_s {
    struct Watcher_s;
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