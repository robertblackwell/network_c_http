#ifndef C_HTTP_RL_CHECKTAG_H
#define C_HTTP_RL_CHECKTAG_H
#include <rbl/check_tag.h>
#include <stdbool.h>
#include <string.h>

#define Runloop_TAG       "RUNLOOP"
#define EventFd_TAG       "EVENTFD"
#define STREAM_TAG        "STREAM"
#define Listener_TAG      "LISTNR"
#define QueueWatcher_TAG  "QWatcher"
#define Timer_TAG         "Timer"
#define ITQueue_TAG       "ITQUEUE"
#define FdTable_TAG       "FDTABL"
#define FunctorList_TAG   "FUNCLST"
#define AsioStream_TAG    "AsioSTRM"

#define WATCHER_DECLARE_TAG RBL_DECLARE_TAG(Watcher)
/**
 * REACTOR tag macros
 */
#define REACTOR_DECLARE_TAG RBL_DECLARE_TAG(Runloop_TAG)
#define RUNLOOP_CHECK_TAG(p) RBL_CHECK_TAG(Runloop_TAG, p)
#define RUNLOOP_SET_TAG(p) RBL_SET_TAG(Runloop_TAG, p)

#define RUNLOOP_DECLARE_END_TAG RBL_DECLARE_END_TAG(Runloop_TAG)
#define RUNLOOP_CHECK_END_TAG(p) RBL_CHECK_END_TAG(Runloop_TAG, p)
#define RUNLOOP_SET_END_TAG(p) RBL_SET_END_TAG(Runloop_TAG, p)
#define VERIFY_RUNLOOP(p) do{\
    RUNLOOP_CHECK_TAG(p) \
    RUNLOOP_CHECK_END_TAG(p) \
}while(0);

/**
 * eventfd tag macros
 */
#define EVENTFD_DECLARE_TAG RBL_DECLARE_TAG(EventFd_TAG)
#define EVENTFD_CHECK_TAG(p) RBL_CHECK_TAG(EventFd_TAG, p)
#define EVENTFD_SET_TAG(p) RBL_SET_TAG(EventFd_TAG, p)

#define EVENTFD_DECLARE_END_TAG RBL_DECLARE_END_TAG(EventFd_TAG)
#define EVENTFD_CHECK_END_TAG(p) RBL_CHECK_END_TAG(EventFd_TAG, p)
#define EVENTFD_SET_END_TAG(p) RBL_SET_END_TAG(EventFd_TAG, p)

/**
 * RunloopStream
 */
#define SOCKW_DECLARE_TAG RBL_DECLARE_TAG(STREAM_TAG)
#define SOCKW_CHECK_TAG(p) RBL_CHECK_TAG(STREAM_TAG, p)
#define SOCKW_SET_TAG(p) RBL_SET_TAG(STREAM_TAG, p)

#define SOCKW_DECLARE_END_TAG RBL_DECLARE_END_TAG(STREAM_TAG)
#define SOCKW_CHECK_END_TAG(p) RBL_CHECK_END_TAG(STREAM_TAG, p)
#define SOCKW_SET_END_TAG(p) RBL_SET_END_TAG(STREAM_TAG, p)

/**
 * WListener
 */
#define LISTNER_DECLARE_TAG RBL_DECLARE_TAG(Listener_TAG)
#define LISTNER_CHECK_TAG(p) RBL_CHECK_TAG(Listener_TAG, p)
#define LISTNER_SET_TAG(p) RBL_SET_TAG(Listener_TAG, p)

#define LISTNER_DECLARE_END_TAG RBL_DECLARE_END_TAG(Listener_TAG)
#define LISTNER_CHECK_END_TAG(p) RBL_CHECK_END_TAG(Listener_TAG, p)
#define LISTNER_SET_END_TAG(p) RBL_SET_END_TAG(Listener_TAG, p)

/**
 * RunloopQueueWatcher
 */
#define QUEUE_WATCHER_DECLARE_TAG RBL_DECLARE_TAG(QueueWatcher_TAG)
#define QUEUE_WATCHER_CHECK_TAG(p) RBL_CHECK_TAG(QueueWatcher_TAG, p)
#define QUEUE_WATCHER_SET_TAG(p) RBL_SET_TAG(QueueWatcher_TAG, p)

#define QUEUE_WATCHER_DECLARE_END_TAG RBL_DECLARE_END_TAG(QueueWatcher_TAG)
#define QUEUE_WATCHER_CHECK_END_TAG(p) RBL_CHECK_END_TAG(QueueWatcher_TAG, p)
#define QUEUE_WATCHER_SET_END_TAG(p) RBL_SET_END_TAG(QueueWatcher_TAG, p)
#define QUEUE_WATCHER_SET_END_TAG(p) RBL_SET_END_TAG(QueueWatcher_TAG, p)

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
#define TIMER_DECLARE_TAG RBL_DECLARE_TAG(Timer_TAG)
#define TIMER_CHECK_TAG(p) RBL_CHECK_TAG(Timer_TAG, p)
#define TIMER_SET_TAG(p) RBL_SET_TAG(Timer_TAG, p)

#define TIMER_DECLARE_END_TAG RBL_DECLARE_EMD_TAG(Timer_TAG)
#define TIMER_CHECK_END_TAG(p) RBL_CHECK_END_TAG(Timer_TAG, p)
#define TIMER_SET_END_TAG(p) RBL_SET_END_TAG(Timer_TAG, p)

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