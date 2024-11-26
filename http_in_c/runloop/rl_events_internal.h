#ifndef C_HTTP_RL_EVENTS_INTERNAL_H
#define C_HTTP_RL_EVENTS_INTERNAL_H
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_checktag.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <http_in_c/common/list.h>
/**
 * The structs defined in this file are all associated with file descriptor events.
 *
 * There is a base struct called RuloopWatcherBase and then a number of sub structs that inherit from
 * the base struct.
 *
 * The way the inheritence is implemented is to include the base struct as an anonymous struct at the start
 * of each deribved struct. This has the effect that a pointer to a derived struct is automatically also
 * a pointer to the base struct. This attribute of events is used relatively widely throoughout this project.
 *
 * One of the side effects of this form of inheritence is the way it interacts with the use of start and
 * end tags to provide error checking against mis-interpreting pointers.
 *
 * The opening tag must be declarred in the base struct and the closing tag declared in the derived struct.
 */
/**
 * RunloopWatcherBase - a generic observer object
 */
typedef enum WatcherType {
    RUNLOOP_WATCHER_SOCKET = 11,
    RUNLOOP_WATCHER_TIMER = 12,
    RUNLOOP_WATCHER_QUEUE = 13,
    RUNLOOP_WATCHER_FDEVENT = 14,
    RUNLOOP_WATCHER_LISTENER = 15,
} WatcherType;


struct RunloopWatcherBase_s {
    RBL_DECLARE_TAG;
    WatcherType           type;
    RunloopRef            runloop;
    void*                 context;
    int                   fd;
    /**
     * function that knows how to free the specific sub type of RunloopWatcherBase from a general ref.
     * Each derived type must provide this function when an instance is created or initializez.
     * In the case of RunloopTimerfd and RunloopEventfd watchers must also close the fd
     */
    void(*free)(RunloopWatcherBaseRef);
    /**
     * first level handler function
     * each derived type provides thier own type specific handler when an instance is created
     * or initialized and must cast the first parameter to their own specific type of watcher
     * inside the handler.
     *
     * This handler will be calledd directly from the epoll_wait code inside runloop.c
    */
    void(*handler)(RunloopWatcherBaseRef watcher_ref, uint64_t event);
};

typedef uint64_t WEventFdMask;
struct RunloopEventfd_s {
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
    PostableFunction    fdevent_postable;
    void*               fdevent_postable_arg;
    int                 write_fd;
    RBL_DECLARE_END_TAG;
};

/**
 * RunloopStream
 */
struct RunloopStream_s {
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
    uint64_t                 event_mask;

    PostableFunction         read_postable_cb;
    void*                    read_postable_arg;
    PostableFunction         write_postable_cb;
    void*                    write_postable_arg;
    RBL_DECLARE_END_TAG;
};

/**
 * WListener
 */
typedef struct RunloopListener_s {
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
    PostableFunction         listen_postable;
    void*                    listen_postable_arg;
    RBL_DECLARE_END_TAG;
} RunloopListener;

/**
 * RunloopQueueWatcher
 */
typedef uint64_t RunloopQueueEvent;
typedef void(RunloopQueuetWatcherCallerback(void* ctx));
struct RunloopQueueWatcher_s {
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
    /**
     * This is a non-owning reference as it was passed in during creation
     * of a RunloopQueueWatcher object.
     */
    EventfdQueueRef            queue;
    // runloop cb and arg
    PostableFunction       queue_postable;
    void*                  queue_postable_arg;

    QueueWatcherReadCallbackFunction read_cb;
    void*                            read_cb_arg;

    RBL_DECLARE_END_TAG;
};

/**
 * InterThreadQueue
 */
typedef uint64_t RunloopInterthreadQueueEvent;
typedef void(RunloopInterthreadQueuetWatcherCallerback(void* ctx));
struct InterthreadQueue_s {
    /** This struct does not inherit from WatcherBase hence must declare its own openning tag*/
    RBL_DECLARE_TAG; 
    EventfdQueueRef queue;
    RunloopRef runloop;
    RunloopQueueWatcherRef qwatcher_ref;
    RBL_DECLARE_END_TAG;
};// InterthreadQueue_s;, InterthreadQueue, *InterthreadQueueRef;

/**
 * RunloopTimer
 */
typedef uint64_t RunloopTimerEvent;
struct RunloopTimer_s {
    /** The start tag is declared in the base struct
    RBL_DECLARE_TAG; */
    struct RunloopWatcherBase_s;
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    PostableFunction        timer_postable;
    void*                   timer_postable_arg;
    RBL_DECLARE_END_TAG;
};

#endif