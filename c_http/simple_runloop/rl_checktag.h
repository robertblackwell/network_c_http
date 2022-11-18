#ifndef rl_tagcheck_h
#define rl_tagcheck_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <stdbool.h>
#include <string.h>


#ifdef TYPE_CHECK_ON
#define TAG_LENGTH 10
#define DECLARE_TAG(TAG) char tag[TAG_LENGTH]
#define CHECK_TAG(TAG, p) \
    do { \
        if(strcmp((p)->tag, TAG) != 0) { \
            assert(false);                  \
        } \
    } while(0);

// used for testing only
#define FAIL_CHECK_TAG(TAG, p) \
    do { \
        assert(strcmp((p)->tag, TAG) != 0); \
    } while(0);

#define SET_TAG(TAG, p) \
    do {                     \
        static_assert(strlen(TAG) < TAG_LENGTH, "Tag too long in SET_TAG");                     \
        sprintf((p)->tag, "%s", TAG); \
    } while(0);
#else
    #define DECLARE_TAG(TYPE)
    #define CHECK_TAG(TYPE, p)
    #define SET_TAG(TYPE, p)
#endif

#define Reactor_TAG     "XREACT"
#define WEventFd_TAG    "XRFDEV"
#define WIoFd_TAG       "XRWIOS"
#define WListenerFd_TAG "XRLSTNR"
#define WQueue_TAG      "XRQUE"
#define WTimerFd_TAG    "XRTIMER"
#define ITQueue_TAG         "ITQUEUE"

#define XR_WATCHER_DECLARE_TAG DECLARE_TAG(Watcher)

#define XR_REACTOR_DECLARE_TAG DECLARE_TAG(Reactor_TAG)
#define XR_REACTOR_CHECK_TAG(p) CHECK_TAG(Reactor_TAG, p)
#define XR_REACTOR_SET_TAG(p) SET_TAG(Reactor_TAG, p)

#define XR_FDEV_DECLARE_TAG DECLARE_TAG(WEventFd_TAG)
#define XR_FDEV_CHECK_TAG(p) CHECK_TAG(WEventFd_TAG, p)
#define XR_FDEV_SET_TAG(p) SET_TAG(WEventFd_TAG, p)

/**
 * RtorRdrWrtr
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
 * WQueue
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

#endif