#ifndef c_http_xr_runloop_h
#define c_http_xr_runloop_h

#include <stdint.h>
#include <time.h>
#include <c_http/runloop/watcher.h>


#define TYPE Reactor
#define Reactor_TAG "XRLRTOT"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_REACTOR_DECLARE_TAG DECLARE_TAG(Reactor)
#define XR_REACTOR_CHECK_TAG(p) CHECK_TAG(Reactor, p)
#define XR_REACTOR_SET_TAG(p) SET_TAG(Reactor, p)

typedef struct XrReactor_s XrReactor, *XrReactorRef;

XrReactorRef XrReactor_new(void);

void XrReactor_free(XrReactorRef rtor_ref);

int XrReactor_register(XrReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);

int XrReactor_deregister(XrReactorRef rtor_ref, int fd);

int XrReactor_reregister(XrReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);

int XrReactor_run(XrReactorRef rtor_ref, time_t timeout);

int XrReactor_post(XrReactorRef rtor_ref, PostableFunction cb, void* arg);
/**
 * Remove an fd and its associated Watcher from the Reactor fd list - BUT does not perform
 * an epoll_ctl DEL operation.
 *
 * This is an alternative to Reactor_deregister for WListener objects.
 *
 * This function is specifically for use by a WListener only - such objects use the EPOLLEXCLUSIVE flag
 * In the presense of EPOLLEXCLUSIVE all epoll_ctl calls after the first return an error with errno == 9
 *
 * @param this XrReactor
 * @param fd   int  file descriptor
 */
void XrReactor_delete(XrReactorRef this, int fd);

#endif
