#ifndef c_http_listener_watcher_h
#define c_http_listener_watcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/aio_api/types.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/watcher.h>

#define TYPE WListener
#define WListener_TAG "XRLST"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_LIST_DECLARE_TAG DECLARE_TAG(WListener)
#define XR_LIST_CHECK_TAG(p) CHECK_TAG(WListener, p)
#define XR_LIST_SET_TAG(p) SET_TAG(WListener, p)


typedef uint64_t XrSocketEvent;

typedef void(WListenerCaller(void* ctx));

struct WListener_s {
    struct Watcher_s;
    ListenerEventHandler*      listen_evhandler;
    void*                    listen_arg;


};

WListenerRef WListener_new(XrReactorRef runloop, int fd);
void WListener_free(WListenerRef this);
void WListener_register(WListenerRef this, ListenerEventHandler event_handler, void* arg);
void WListener_deregister(WListenerRef this);

/**
 * Enable reception of fd listener events and set the event handler
 * @param this            WListenerRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param fd_event_handler
 */
void WListener_arm(WListenerRef this, ListenerEventHandler fd_event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void XrLIstener_disarm(WListenerRef this);

#endif