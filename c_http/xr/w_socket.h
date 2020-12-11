#ifndef c_http_socketwatcher_h
#define c_http_socketwatcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>

#define TYPE WSocket
#define WSocket_TAG "XRWS"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_SOCKW_DECLARE_TAG DECLARE_TAG(WSocket)
#define XR_SOCKW_CHECK_TAG(p) CHECK_TAG(WSocket, p)
#define XR_SOCKW_SET_TAG(p) SET_TAG(WSocket, p)

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

typedef void(WSocketCaller(void* ctx));

struct WSocket_s {
    struct Watcher_s;
//    SocketEventHandler* cb;
//    void*                    cb_ctx;
    uint64_t                 event_mask;
    SocketEventHandler*      read_evhandler;
    void*                    read_arg;
    SocketEventHandler*      write_evhandler;
    void*                    write_arg;


};

WSocketRef WSocket_new(XrReactorRef runloop, int fd);
void WSocket_free(WSocketRef this);
void WSocket_register(WSocketRef this);
void WSocket_deregister(WSocketRef this);
/**
 * Enable reception of fd read events and set the event handler
 * @param this            WSocketRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param event_handler
 */
void WSocket_arm_read(WSocketRef this, SocketEventHandler event_handler, void* arg);
/**
 * Enable reception of fd write events and set the event handler
 * @param this            WSocketRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param event_handler
 */
void WSocket_arm_write(WSocketRef this, SocketEventHandler event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void WSocket_disarm_read(WSocketRef this);

/**
 * Disable reception of fd write events for the socket
 * @param this
 */
void WSocket_disarm_write(WSocketRef this);


#endif