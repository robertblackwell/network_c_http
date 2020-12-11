#ifndef c_http_socketwatcher_h
#define c_http_socketwatcher_h
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>

#define TYPE XrSockW
#define XrSockW_TAG "XRWS"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_SOCKW_DECLARE_TAG DECLARE_TAG(XrSockW)
#define XR_SOCKW_CHECK_TAG(p) CHECK_TAG(XrSockW, p)
#define XR_SOCKW_SET_TAG(p) SET_TAG(XrSockW, p)

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

typedef void(XrSocketWatcherCallback(XrWatcherRef watch, void* arg, uint64_t event));
typedef void(XrSocketWatcherCaller(void* ctx));

struct XrSocketWatcher_s {
    struct XrWatcher_s;
//    XrSocketWatcherCallback* cb;
//    void*                    cb_ctx;
    uint64_t                 event_mask;
    XrSocketWatcherCallback* read_evhandler;
    void*                    read_arg;
    XrSocketWatcherCallback* write_evhandler;
    void*                    write_arg;


};

XrSocketWatcherRef Xrsw_new(XrReactorRef runloop, int fd);
void Xrsw_free(XrSocketWatcherRef this);
void Xrsw_register(XrSocketWatcherRef this);
//void Xrsw_change_watch(XrSocketWatcherRef this, XrSocketWatcherCallback cb, void* arg, uint64_t watch_what);
void Xrsw_deregister(XrSocketWatcherRef this);
/**
 * Enable reception of fd read events and set the event handler
 * @param this            XrSocketWatcherRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param fd_event_handler
 */
void Xrsw_arm_read(XrSocketWatcherRef this, XrSocketWatcherCallback fd_event_handler, void* arg);
/**
 * Enable reception of fd write events and set the event handler
 * @param this            XrSocketWatcherRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param fd_event_handler
 */
void Xrsw_arm_write(XrSocketWatcherRef this, XrSocketWatcherCallback fd_event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void Xrsw_disarm_read(XrSocketWatcherRef this);

/**
 * Disable reception of fd write events for the socket
 * @param this
 */
void Xrsw_disarm_write(XrSocketWatcherRef this);


#endif