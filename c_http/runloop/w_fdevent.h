#ifndef c_http_fdevent_h
#define c_http_fdevent_h
#include <time.h>
#include <stdint.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/reactor.h>

#define TYPE WFdEvent
#define WFdEvent_TAG "XRFDEV"
#include <c_http/check_tag.h>
#undef TYPE
#define XR_FDEV_DECLARE_TAG DECLARE_TAG(WFdEvent)
#define XR_FDEV_CHECK_TAG(p) CHECK_TAG(WFdEvent, p)
#define XR_FDEV_SET_TAG(p) SET_TAG(WFdEvent, p)

/**
 * An WFdEvent is a generic type of event that can be waited on and fired. It is intended to provide
 * a generic async signalling mechanism between threads within a process that uses epoll_wait. This last point
 * is important as it uniformizes event waiting within an application.
 *
 * Under the covers there are two alternative mechanisms:
 *
 * -    the two pipe trick. A pipe is created and a thread can wait on
 *      an epoll read event. the event is fired by writing to the write end of the pipe
 *      which causes epoll_wait to retrun a read even for the read end of the pipe.
 *
 * -    Linux eventfd which has two modes:
 *      -   semaphore mode
 *      -   non semaphore mode
 *      read eventfd man page for details.
 *
 *      Note the two-pipe-trick behaviour is consistent with eventfd in semaphore mode.
 *
 *      If you run test_eventfd the 2-pipe-trick and eventfd+semaphore will work
 *      but eventfd without semaphore will fail.
 *      You config such runs with the #defines in w_fdevent.c
 *      Why this is so is left as an exercise to the reader.
 *      It has to do with how multiple writes to the fd arer handled.
 *
 *      The 2pipetrick writes multilple 64bit quantities to the pipe so multiple reads succeed
 *      In semaphore mode
 *          a write of 1 increments the value in the fd
 *          a read decrements the value in the fd
 *      In non semaphore mode a read empties the fd regardless of the value
 *
 * The current setting is eventfd+semaphore. The result is a semaphore mechanism that operates via epoll.
 *
 * This "type" is a derived type with Watcher as the base.
 *
 */
struct WFdEvent_s;
typedef struct WFdEvent_s WFdEvent, *WFdEventRef;
typedef uint64_t WFdEventMask;

typedef void(WFdEventCaller(void* ctx));

struct WFdEvent_s {
    struct Watcher_s;

    FdEventHandler*     fd_event_handler;
    void*               fd_event_handler_arg;
    int                 write_fd;
};

WFdEventRef WFdEvent_new(XrReactorRef runloop);
void WFdEvent_free(WFdEventRef this);
void WFdEvent_register(WFdEventRef this);
void WQueue_change_watch(WFdEventRef this, FdEventHandler evhandler, void* arg, uint64_t watch_what);

void WFdEvent_arm(WFdEventRef this,  FdEventHandler evhandler, void* arg);
void WFdEvent_disarm(WFdEventRef this);
void WFdEvent_fire(WFdEventRef this);
void WFdEvent_deregister(WFdEventRef this);
void WFdEvent_fire(WFdEventRef this);




#endif