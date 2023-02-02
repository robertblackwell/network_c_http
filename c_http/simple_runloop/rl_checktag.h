#ifndef C_HTTP_RL_CHECKTAG_H
#define C_HTTP_RL_CHECKTAG_H
#include <c_http/check_tag.h>
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

#define WATCHER_DECLARE_TAG DECLARE_TAG(Watcher)

#define REACTOR_DECLARE_TAG DECLARE_TAG(Reactor_TAG)
#define REACTOR_CHECK_TAG(p) CHECK_TAG(Reactor_TAG, p)
#define REACTOR_SET_TAG(p) SET_TAG(Reactor_TAG, p)

#define FDEV_DECLARE_TAG DECLARE_TAG(WEventFd_TAG)
#define FDEV_CHECK_TAG(p) CHECK_TAG(WEventFd_TAG, p)
#define FDEV_SET_TAG(p) SET_TAG(WEventFd_TAG, p)

/**
 * RtorStream
 */
#define SOCKW_DECLARE_TAG DECLARE_TAG(WIoFd_TAG)
#define SOCKW_CHECK_TAG(p) CHECK_TAG(WIoFd_TAG, p)
#define SOCKW_SET_TAG(p) SET_TAG(WIoFd_TAG, p)

/**
 * WListener
 */
#define LISTNER_DECLARE_TAG DECLARE_TAG(WListenerFd_TAG)
#define LISTNER_CHECK_TAG(p) CHECK_TAG(WListenerFd_TAG, p)
#define LISTNER_SET_TAG(p) SET_TAG(WListenerFd_TAG, p)

/**
 * RtorWQueue
 */
#define WQUEUE_DECLARE_TAG DECLARE_TAG(WQueue_TAG)
#define WQUEUE_CHECK_TAG(p) CHECK_TAG(WQueue_TAG, p)
#define WQUEUE_SET_TAG(p) SET_TAG(WQueue_TAG, p)

/**
 * ITQueue
 */
#define ITQUEUE_DECLARE_TAG DECLARE_TAG(ITQueue_TAG)
#define ITQUEUE_CHECK_TAG(p) CHECK_TAG(ITQueue_TAG, p)
#define ITQUEUE_SET_TAG(p) SET_TAG(ITQueue_TAG, p)


/**
 * RtorTimer
 */
#define WTIMER_DECLARE_TAG DECLARE_TAG(WTimerFd_TAG)
#define WTIMER_CHECK_TAG(p) CHECK_TAG(WTimerFd_TAG, p)
#define WTIMER_SET_TAG(p) SET_TAG(WTimerFd_TAG, p)
/**
 * FdTable
 */
#define FDTABL_DECLARE_TAG DECLARE_TAG(FdTable_TAG)
#define FDTABL_CHECK_TAG(p) CHECK_TAG(FdTable_TAG, p)
#define FDTABL_SET_TAG(p) SET_TAG(FdTable_TAG, p)

/**
 * Functor List
 */
#define FNCLST_DECLARE_TAG DECLARE_TAG(FunctorList_TAG)
#define FNCLST_CHECK_TAG(p) CHECK_TAG(FunctorList_TAG, p)
#define FNCLST_SET_TAG(p) SET_TAG(FunctorList_TAG, p)


#endif