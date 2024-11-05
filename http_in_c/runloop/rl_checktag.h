#ifndef C_HTTP_RL_CHECKTAG_H
#define C_HTTP_RL_CHECKTAG_H
#include <rbl/check_tag.h>
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
#define AsioStream_TAG "AsioSTRM"

#define WATCHER_DECLARE_TAG RBL_DECLARE_TAG(Watcher)

/**
 * REACTOR tag macros
 */
#define REACTOR_DECLARE_TAG RBL_DECLARE_TAG(Reactor_TAG)
#define REACTOR_CHECK_TAG(p) RBL_CHECK_TAG(Reactor_TAG, p)
#define REACTOR_SET_TAG(p) RBL_SET_TAG(Reactor_TAG, p)

#define REACTOR_DECLARE_END_TAG RBL_DECLARE_END_TAG(Reactor_TAG)
#define REACTOR_CHECK_END_TAG(p) RBL_CHECK_END_TAG(Reactor_TAG, p)
#define REACTOR_SET_END_TAG(p) RBL_SET_TAG(Reactor_TAG, p)

/**
 * eventfd tag macros
 */
#define FDEV_DECLARE_TAG RBL_DECLARE_TAG(WEventFd_TAG)
#define FDEV_CHECK_TAG(p) RBL_CHECK_TAG(WEventFd_TAG, p)
#define FDEV_SET_TAG(p) RBL_SET_TAG(WEventFd_TAG, p)

#define FDEV_DECLARE_END_TAG RBL_DECLARE_END_TAG(WEventFd_TAG)
#define FDEV_CHECK_END_TAG(p) RBL_CHECK_END_TAG(WEventFd_TAG, p)
#define FDEV_SET_END_TAG(p) RBL_SET_END_TAG(WEventFd_TAG, p)

/**
 * RunloopStream
 */
#define SOCKW_DECLARE_TAG RBL_DECLARE_TAG(WIoFd_TAG)
#define SOCKW_CHECK_TAG(p) RBL_CHECK_TAG(WIoFd_TAG, p)
#define SOCKW_SET_TAG(p) RBL_SET_TAG(WIoFd_TAG, p)

#define SOCKW_DECLARE_END_TAG RBL_DECLARE_END_TAG(WIoFd_TAG)
#define SOCKW_CHECK_END_TAG(p) RBL_CHECK_END_TAG(WIoFd_TAG, p)
#define SOCKW_SET_END_TAG(p) RBL_SET_END_TAG(WIoFd_TAG, p)

/**
 * WListener
 */
#define LISTNER_DECLARE_TAG RBL_DECLARE_TAG(WListenerFd_TAG)
#define LISTNER_CHECK_TAG(p) RBL_CHECK_TAG(WListenerFd_TAG, p)
#define LISTNER_SET_TAG(p) RBL_SET_TAG(WListenerFd_TAG, p)

#define LISTNER_DECLARE_END_TAG RBL_DECLARE_END_TAG(WListenerFd_TAG)
#define LISTNER_CHECK_END_TAG(p) RBL_CHECK_END_TAG(WListenerFd_TAG, p)
#define LISTNER_SET_END_TAG(p) RBL_SET_END_TAG(WListenerFd_TAG, p)

/**
 * RunloopQueueWatcher
 */
#define WQUEUE_DECLARE_TAG RBL_DECLARE_TAG(WQueue_TAG)
#define WQUEUE_CHECK_TAG(p) RBL_CHECK_TAG(WQueue_TAG, p)
#define WQUEUE_SET_TAG(p) RBL_SET_TAG(WQueue_TAG, p)

#define WQUEUE_DECLARE_END_TAG RBL_DECLARE_END_TAG(WQueue_TAG)
#define WQUEUE_CHECK_END_TAG(p) RBL_CHECK_END_TAG(WQueue_TAG, p)
#define WQUEUE_SET_END_TAG(p) RBL_SET_END_TAG(WQueue_TAG, p)

/**
 * ITQueue
 */
#define ITQUEUE_DECLARE_TAG RBL_DECLARE_TAG(ITQueue_TAG)
#define ITQUEUE_CHECK_TAG(p) RBL_CHECK_TAG(ITQueue_TAG, p)
#define ITQUEUE_SET_TAG(p) RBL_SET_TAG(ITQueue_TAG, p)

#define ITQUEUE_DECLARE_END_TAG RBL_DECLARE_END_TAG(ITQueue_TAG)
#define ITQUEUE_CHECK_END_TAG(p) RBL_CHECK_END_TAG(ITQueue_TAG, p)
#define ITQUEUE_SET_END_TAG(p) RBL_SET_END_TAG(ITQueue_TAG, p)

/**
 * RunloopTimer
 */
#define WTIMER_DECLARE_TAG RBL_DECLARE_TAG(WTimerFd_TAG)
#define WTIMER_CHECK_TAG(p) RBL_CHECK_TAG(WTimerFd_TAG, p)
#define WTIMER_SET_TAG(p) RBL_SET_TAG(WTimerFd_TAG, p)

#define WTIMER_DECLARE_END_TAG RBL_DECLARE_EMD_TAG(WTimerFd_TAG)
#define WTIMER_CHECK_END_TAG(p) RBL_CHECK_END_TAG(WTimerFd_TAG, p)
#define WTIMER_SET_END_TAG(p) RBL_SET_END_TAG(WTimerFd_TAG, p)

/**
 * FdTable
 */
#define FDTABL_DECLARE_TAG RBL_DECLARE_TAG(FdTable_TAG)
#define FDTABL_CHECK_TAG(p) RBL_CHECK_TAG(FdTable_TAG, p)
#define FDTABL_SET_TAG(p) RBL_SET_TAG(FdTable_TAG, p)

#define FDTABL_DECLARE_END_AG RBL_DECLARE_END_TAG(FdTable_TAG)
#define FDTABL_CHECK_END_TAG(p) RBL_CHECK_END_TAG(FdTable_TAG, p)
#define FDTABL_SET_END_TAG(p) RBL_SET_END_TAG(FdTable_TAG, p)

/**
 * Functor List
 */
#define FNCLST_DECLARE_TAG RBL_DECLARE_TAG(FunctorList_TAG)
#define FNCLST_CHECK_TAG(p) RBL_CHECK_TAG(FunctorList_TAG, p)
#define FNCLST_SET_TAG(p) RBL_SET_TAG(FunctorList_TAG, p)

#define FNCLST_DECLARE_END_TAG RBL_DECLARE_END_TAG(FunctorList_TAG)
#define FNCLST_CHECK_END_TAG(p) RBL_CHECK_END_TAG(FunctorList_TAG, p)
#define FNCLST_SET_END_TAG(p) RBL_SET_END_TAG(FunctorList_TAG, p)

#endif