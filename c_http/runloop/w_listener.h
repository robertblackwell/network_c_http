#ifndef c_http_listener_watcher_h
#define c_http_listener_watcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/runloop/types.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/reactor.h>


#define TYPE WListenerFd
#define WListenerFd_TAG "XRLSTNR"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_LISTNER_DECLARE_TAG DECLARE_TAG(WListenerFd)
#define XR_LISTNER_CHECK_TAG(p) CHECK_TAG(WListenerFd, p)
#define XR_LISTNER_SET_TAG(p) SET_TAG(WListenerFd, p)


// typedef uint64_t XrSocketEvent;

// typedef void(WListenerCaller(void* ctx));

struct WListenerFd_s {
    struct Watcher_s;
    ListenerEventHandler*    listen_evhandler;
    void*                    listen_arg;
};

WListenerFdRef WListenerFd_new(ReactorRef runloop, int fd);
void WListenerFd_free(WListenerFdRef this);
void WListenerFd_register(WListenerFdRef this, ListenerEventHandler event_handler, void* arg);
void WListenerFd_deregister(WListenerFdRef this);

/**
 * Enable reception of fd listener events and set the event handler
 * @param this            WListenerFdRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param fd_event_handler
 */
void WListenerFd_arm(WListenerFdRef this, ListenerEventHandler fd_event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void XrLIstener_disarm(WListenerFdRef this);

#endif