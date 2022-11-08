#ifndef c_http_iofd_watcher_h
#define c_http_iofd_watcher_h
#if 0
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <c_http/simple_runloop/types.h>
#include <c_http/simple_runloop/watcher.h>
#include <c_http/simple_runloop/reactor.h>


#define TYPE WIoFd
#define WIoFd_TAG "XRWS"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_SOCKW_DECLARE_TAG DECLARE_TAG(WIoFd)
#define XR_SOCKW_CHECK_TAG(p) CHECK_TAG(WIoFd, p)
#define XR_SOCKW_SET_TAG(p) SET_TAG(WIoFd, p)


// typedef void(WSocketCaller(void* ctx));

struct WIoFd_s {
    struct Watcher_s;

    uint64_t                 event_mask;
    SocketEventHandler*      read_evhandler;
    void*                    read_arg;
    SocketEventHandler*      write_evhandler;
    void*                    write_arg;


};

WIoFdRef WIoFd_new(ReactorRef runloop, int fd);
void WIoFd_free(WIoFdRef athis);

void WIoFd_register(WIoFdRef athis);
void WIoFd_deregister(WIoFdRef athis);

/**
 * Enable reception of fd read events and set the event handler
 * @param this            WIoFdRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param event_handler
 */
void WIoFd_arm_read(WIoFdRef athis, SocketEventHandler event_handler, void* arg);

/**
 * Enable reception of fd write events and set the event handler
 * @param this            WIoFdRef
 * @param arg             void*               Context data. Discretion of the caller
 * @param event_handler
 */
void WIoFd_arm_write(WIoFdRef athis, SocketEventHandler event_handler, void* arg);

/**
 * Disable reception of fd read events for the socket
 * @param this
 */
void WIoFd_disarm_read(WIoFdRef athis);

/**
 * Disable reception of fd write events for the socket
 * @param this
 */
void WIoFd_disarm_write(WIoFdRef athis);

#endif
#endif