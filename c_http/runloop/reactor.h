#ifndef c_http_xr_runloop_h
#define c_http_xr_runloop_h
#define _GNU_SOURCE
#include <stdint.h>
#include <time.h>
#include <sys/epoll.h>
#include <c_http/runloop/watcher.h>

#define TYPE Reactor
#define Reactor_TAG "XRLRTOT"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_REACTOR_DECLARE_TAG DECLARE_TAG(Reactor)
#define XR_REACTOR_CHECK_TAG(p) CHECK_TAG(Reactor, p)
#define XR_REACTOR_SET_TAG(p) SET_TAG(Reactor, p)

/**
 * An opaque struct that represents a reactor. 
 *
 *  A reactor is an object that waits for events associated with fds(file descriptors)
 *  and also manages a runloop so that eveant handling code can "yield" the processor
 *  but post a callback function (function pointer and void* context pointer)
 *  to continue their processing after others have had a go.
 *  
 */
typedef struct XrReactor_s XrReactor, *XrReactorRef;


/**
 * Create a new reactor using dynamic memory
 */ 
XrReactorRef XrReactor_new(void);

/**
 * Close a reactor. This call closes the epoll fd, closes all the fds in the FDTable
 * and flags the XrReactor as closed.
 * This will force an error return from epoll_wait. This will not be reported as an error
 * because of the close flag == true
 *
 * @param XrReactorRef stor_ref
 */
void XrReactor_close(XrReactorRef rtor_ref);
/**
 * Destroy a reactor including release all fds associated with the reactor
 */
void XrReactor_free(XrReactorRef rtor_ref);

/**
 * Register an fd with the reactor. The fd is then "of interest" and the reactor will
 * call the wref handler when an event of interest occurs on the fd.
 * 
 * Registration involves a system call and hence is expensive
 * 
 * @param XrReactorRef rtor_ref  A reference to the subject reactor
 * @param int          fd        subject File descriptor
 * @param uint32_t     interest  mask of events of interest for this fd
 * @param WatcherRef   wref      A reference to an fd Watcher. See watcher.h and w_xxxx.h
 * @return int always 0 TODO should return void
 */
int XrReactor_register(XrReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);

/**
 * Deregister an fd with the reactor. Removes the fd from the field of interest for the reactor.
 * After this call the reactor will not invoke the Watcher parameter and the fd is no longer
 * in the Reactors FdTable.
 * 
 * Deregistration involves a system call and hence is expensive
 * 
 * @param XrReactorRef rtor_ref  A reference to the subject reactor
 * @param int          fd        subject File descriptor
 * @return int always 0 TODO should return void
 */
int XrReactor_deregister(XrReactorRef rtor_ref, int fd);

/**
 * TODO should be deprecated OR need a lightweight deregister
 */
int XrReactor_reregister(XrReactorRef rtor_ref, int fd, uint32_t interest, WatcherRef wref);

/**
 * Waits for events. When an fd event is triggered for a registered fd 
 * runs the associated Watcher. 
 * 
 * When all events have been services runs all "functors" on the run list.
 * 
 * @param XrReactorRef rtor_ref     A reference to the subject reactor
 * @param time_t       timeout      Maximum time in milli secs that will wait for 
 *                                  events before runniing the run list
 * @return int always 0 - TODO should return void
 */
int XrReactor_run(XrReactorRef rtor_ref, time_t timeout);

/**
 * TODO - should replace PostableFunction cb, void* arg with a type called PostableFunctor
 * 
 * Adds a function and context (void*) to the runlist associated with a reactor.
 * 
 * @param XrReactorRef      rtor_ref    A reference to the subject reactor
 * @param PostableFunction  cb          The function to be called with executing
 *                                      this runlist item
 * @param void*             arg         Anonamous parameter passed to cb 
 *  
 * @return int always 0 - TODO should return void
 */
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
