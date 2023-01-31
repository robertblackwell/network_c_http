#ifndef C_HTTP_RL_CHECKTAG_H
#define C_HTTP_RL_CHECKTAG_H
#include <c_http/check_tag.h>
//#include <c_http/simple_runloop/runloop.h>
//#include <c_http/simple_runloop/rl_internal.h>
#include <stdbool.h>
#include <string.h>

#define Reactor_TAG     "XREACT"
#define WEventFd_TAG    "XRFDEV"
#define WIoFd_TAG       "XRWIOS"
#define WListenerFd_TAG "XRLSTNR"
#define WQueue_TAG      "XRQUE"
#define WTimerFd_TAG    "XRTIMER"
#define ITQueue_TAG     "ITQUEUE"
#define FdTable_TAG     "FDTABL"
#define FunctorList_TAG "FUNCLST"

#define XR_WATCHER_DECLARE_TAG DECLARE_TAG(Watcher)

#define XR_REACTOR_DECLARE_TAG DECLARE_TAG(Reactor_TAG)
#define XR_REACTOR_CHECK_TAG(p) CHECK_TAG(Reactor_TAG, p)
#define XR_REACTOR_SET_TAG(p) SET_TAG(Reactor_TAG, p)

#define XR_FDEV_DECLARE_TAG DECLARE_TAG(WEventFd_TAG)
#define XR_FDEV_CHECK_TAG(p) CHECK_TAG(WEventFd_TAG, p)
#define XR_FDEV_SET_TAG(p) SET_TAG(WEventFd_TAG, p)

/**
 * RtorStream
 */
#define XR_SOCKW_DECLARE_TAG DECLARE_TAG(WIoFd_TAG)
#define XR_SOCKW_CHECK_TAG(p) CHECK_TAG(WIoFd_TAG, p)
#define XR_SOCKW_SET_TAG(p) SET_TAG(WIoFd_TAG, p)

/**
 * WListener
 */
#define XR_LISTNER_DECLARE_TAG DECLARE_TAG(WListenerFd_TAG)
#define XR_LISTNER_CHECK_TAG(p) CHECK_TAG(WListenerFd_TAG, p)
#define XR_LISTNER_SET_TAG(p) SET_TAG(WListenerFd_TAG, p)

/**
 * RtorWQueue
 */
#define XR_WQUEUE_DECLARE_TAG DECLARE_TAG(WQueue_TAG)
#define XR_WQUEUE_CHECK_TAG(p) CHECK_TAG(WQueue_TAG, p)
#define XR_WQUEUE_SET_TAG(p) SET_TAG(WQueue_TAG, p)

/**
 * ITQueue
 */
#define XR_ITQUEUE_DECLARE_TAG DECLARE_TAG(ITQueue_TAG)
#define XR_ITQUEUE_CHECK_TAG(p) CHECK_TAG(ITQueue_TAG, p)
#define XR_ITQUEUE_SET_TAG(p) SET_TAG(ITQueue_TAG, p)


/**
 * RtorTimer
 */
#define XRTW_DECLARE_TAG DECLARE_TAG(WTimerFd_TAG)
#define XR_WTIMER_CHECK_TAG(p) CHECK_TAG(WTimerFd_TAG, p)
#define XR_WTIMER_SET_TAG(p) SET_TAG(WTimerFd_TAG, p)
/**
 * FdTable
 */
#define XR_FDTABL_DECLARE_TAG DECLARE_TAG(FdTable_TAG)
#define XR_FDTABL_CHECK_TAG(p) CHECK_TAG(FdTable_TAG, p)
#define XR_FDTABL_SET_TAG(p) SET_TAG(FdTable_TAG, p)

/**
 * Functor List
 */
#define XR_FNCLST_DECLARE_TAG DECLARE_TAG(FunctorList_TAG)
#define XR_FNCLST_CHECK_TAG(p) CHECK_TAG(FunctorList_TAG, p)
#define XR_FNCLST_SET_TAG(p) SET_TAG(FunctorList_TAG, p)


#endif