#ifndef C_HTTP_EPOLL_RL_EVENTS_INTERNAL_H
#define C_HTTP_EPOLL_RL_EVENTS_INTERNAL_H
#include "runloop_internal.h"
#include <runloop/rl_internal.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <common/list.h>
// @TODO complete the merging of rl_events_internal.h
/**
 * The structs defined in this file are all associated with file descriptor events.
 *
 * There is a base struct called RuloopEventBase and then a number of sub structs that inherit from
 * the base struct.
 *
 * The way the inheritence is implemented is to include the base struct as an anonymous struct at the start
 * of each derived struct. This has the effect that a pointer to a derived struct is automatically also
 * a pointer to the base struct. This attribute of events is used relatively widely throughout this project.
 *
 * This technique relies on a compiler extension -fms-extensions and with Apples CLANG compiler produces a warning
 * "-Wmicrosoft-anon-tag" .. below that warning is disabled for this file.
 *
 * In the event that a compiler does not supprt the -fms-extension an alternative method using a macro
 * names -- RUNLOOP_EVENTBASE_Fields -- is provided and is conditionally compiled in or out
 * with the MICROSOFT_ANON_TAG macro
 *
 * One of the side effects of this form of inheritence is the way it interacts with the use of start and
 * end tags to provide error checking against mis-interpreting pointers.
 *
 * The opening tag must be declared in the base struct and the closing tag declared in the derived struct,
 * and the value of both tags given in the derived struct.
 */
/**
 * RunloopWatcherBase - a generic observer object
 */
typedef enum RunloopEventType {
    RUNLOOP_EVENT_SOCKET = 11,
    RUNLOOP_EVENT_TIMER = 12,
    RUNLOOP_EVENT_QUEUE = 13,
    RUNLOOP_USER_EVENT = 14,
    RUNLOOP_EVENT_LISTENER = 15,
    RUNLOOP_EVENT_SIGNAL = 16,
} EventTypeEnum;

#ifndef MICROSOFT_ANON_TAG
// each event has these fields at the start of its struct, it is a form of inheritance
#define RUNLOOP_EVENTBASE_Fields   \
    EventTypeEnum         type;    \
    RunloopRef            runloop; \
    int                   fd;
    void*                 context;  \
    void(*handler)(RunloopEventBaseRef watcher, uint16_t filter, uint16_t flags, void* data);
#endif

struct RunloopEventBase_s {
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    RBL_DECLARE_TAG;
    EventTypeEnum         type;
    RunloopRef            runloop;
    void*                 context;
    int                   fd;
    void(*free)(RunloopEventBaseRef);
    void(*handler)(RunloopEventBaseRef watcher_ref, uint64_t event);
#endif
};
/**
 * RunloopTimer
 */
typedef uint64_t RunloopTimerEvent;

struct RunloopTimer_s {
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
#endif
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    PostableFunction        timer_postable;
    void*                   timer_postable_arg;
    int                     state;
    RBL_DECLARE_END_TAG;
};

/**
 * User event
*/
typedef uint64_t WEventFdMask;
struct RunloopUserEvent_s {
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopEventBase_s;
#endif
    PostableFunction    fdevent_postable;
    void*               fdevent_postable_arg;
    RBL_DECLARE_END_TAG;
};

/**
 * RunloopStream
 */
struct RunloopStream_s {
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
#endif
    uint64_t                 event_mask;
    PostableFunction         read_postable_cb;
    void*                    read_postable_arg;
    PostableFunction         write_postable_cb;
    void*                    write_postable_arg;
    RBL_DECLARE_END_TAG;
};

/**
 * Listener
 */
typedef struct RunloopListener_s {
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
#endif
    PostableFunction         listen_postable;
    void*                    listen_postable_arg;
    RBL_DECLARE_END_TAG;
} RunloopListener;

/**
 * Signal event - catch signals via event queue
 */
struct RunloopSignal_s{
#ifndef MICROSOFT_ANON_TAG
    RBL_DECLARE_TAG;
    RUNLOOP_EVENTBASE_Fields
#else
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopEventBase_s;
#endif
    RBL_DECLARE_END_TAG;
};


typedef struct UserEventQueue_s {
    /** This struct is not a sub struct of Watcher hence it must declare its own openning tag*/
    RBL_DECLARE_TAG;
    FunctorListRef      list;
    pthread_mutex_t     queue_mutex;
    RunloopRef          runloop;
    RunloopUserEventRef user_event;
#ifdef C_HTTP_EFD_QUEUE
#else
    int                 pipefds[2];
#endif
    int                 readfd;
    int                 writefd;
    int                 id;
    RBL_DECLARE_END_TAG;
} UserEventQueue;



#endif