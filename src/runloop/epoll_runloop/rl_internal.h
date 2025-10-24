#ifndef C_HTTP_epoll_RL_INTERNAL_H
#define C_HTTP_epoll_RL_INTERNAL_H
#include <runloop/epoll_runloop/runloop.h>
#include <runloop/epoll_runloop/rl_checktag.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <common/list.h>

#define runloop_MAX_FDS            1024
#define runloop_MAX_RUNLIST        1024
#define runloop_MAX_ITQ            256
#define runloop_MAX_WATCHERS       runloop_MAX_FDS
#define runloop_MAX_EVENTS         runloop_MAX_WATCHERS*2
#define runloop_FUNCTOR_LIST_MAX   runloop_MAX_ITQ
#define runloop_MAX_EPOLL_FDS      runloop_MAX_FDS
#define CBTABLE_MAX                runloop_MAX_FDS
#define runloop_FDTABLE_MAX        runloop_MAX_FDS
#define runloop_READY_LIST_MAX     (2 * runloop_MAX_FDS)

// enables use of eventfd rather than two pipe trick
#define  runloop_eventfd_ENABLE

typedef struct EventTable_s EventTable, *EventTableRef;

struct FdTable_s;
//===============
// #define CBTABLE_MAX 4096

typedef struct FdTable_s FdTable, *FdTableRef;
FdTableRef FdTable_new();
void       FdTable_free(FdTableRef athis);
void       FdTable_insert(FdTableRef athis, RunloopWatcherBaseRef wref, int fd);
void       FdTable_remove(FdTableRef athis, int fd);
RunloopWatcherBaseRef FdTable_lookup(FdTableRef athis, int fd);
int        FdTable_iterator(FdTableRef athis);
int        FdTable_next_iterator(FdTableRef athis, int iter);
uint64_t   FdTable_size(FdTableRef athis);

/**
 * A Functor is a generic callback - a function pointer (of type PostableFunction) and single anonymous argument.
 *
 * The significant thing is that the function pointer, points to a function that has the correct
 * signature for the RunList
 *
*/
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_init(FunctorRef funref, PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis, RunloopRef runloop_ref);
bool Functor_is_empty(FunctorRef f);
void Functor_dealloc(void **p);

typedef struct FunctorList_s {
//    char       tag[RBL_TAG_LENGTH];
    RBL_DECLARE_TAG;
    int        capacity;
    int        head;
    int        tail_plus;
    FunctorRef list; // points to an array of Functor objects
    RBL_DECLARE_END_TAG;
} FunctorList, *FunctorListRef;

/**
 * NOTE: FunctionList acceptS and returnS values of a Functor NOT a pointer
 */
FunctorListRef functor_list_new(int capacity);
void functor_list_free(FunctorListRef this);
void functor_list_add(FunctorListRef this, Functor func);
Functor functor_list_remove(FunctorListRef this);
int functor_list_size(FunctorListRef this);

void fd_map_init();
bool fd_map_at(int j);
bool fd_map_set(int j);

#define REGISTER_WQUEUE_REACTOR 1

struct Runloop_s {
    RBL_DECLARE_TAG;
    int                     epoll_fd;
    bool                    closed_flag;
    bool                    runloop_executing;
    pid_t                   tid;
    EventTableRef           event_table_ref;
    FdTableRef              table; // (int, CallbackData)
    FunctorListRef          ready_list;
    RBL_DECLARE_END_TAG;
};

void* rl_event_allocate(RunloopRef rl, size_t size_required);
void  rl_event_free(RunloopRef rl, void* p);
#if 0
/**
 * This include file holds the definition of structs related to file descriptor events.
 *
 * I have put these definitions in a separate include file to stress their unique
 * attributes and thrie relationship to the base struct and each other.
 */
#include "rl_events_internal.h"

typedef struct EventfdQueue_s {
    /** This struct is not a sub struct of Watcher hence it must declare its own openning tag*/
    RBL_DECLARE_TAG;
    FunctorListRef      list;
    pthread_mutex_t     queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int                 pipefds[2];
#endif
    int                 readfd;
    int                 writefd;
    int                 id;
    RBL_DECLARE_END_TAG;
} EventfdQueue;


typedef struct AsioStream_s {
    /** This struct is diffenrent to most watchers as it is no a sub class of Watcher
     * hence it must declare its own openning tag */
    RBL_DECLARE_TAG;
    int                 fd;
    RunloopStreamRef    runloop_stream_ref;

    int                 read_state;
    void*               read_buffer;
    long                read_buffer_size;
    AsioReadcallback    read_callback;
    void*               read_callback_arg;

    int                 write_state;
    void*               write_buffer;
    long                write_buffer_size;
    AsioWritecallback   write_callback;
    void*               write_callback_arg;
    RBL_DECLARE_END_TAG;
} AsioStream, *AsioStreamRef;

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


typedef struct EventfdQueue_s {
    /** This struct is not a sub struct of Watcher hence it must declare its own openning tag*/
    RBL_DECLARE_TAG;
    FunctorListRef      list;
    pthread_mutex_t     queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int                 pipefds[2];
#endif
    int                 readfd;
    int                 writefd;
    int                 id;
    RBL_DECLARE_END_TAG;
} EventfdQueue;

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

typedef struct AsioStream_s {
    /** This struct is diffenrent to most watchers as it is no a sub class of Watcher
     * hence it must declare its own openning tag */
    RBL_DECLARE_TAG;
    int                 fd;
    RunloopStreamRef    runloop_stream_ref;

    int                 read_state;
    void*               read_buffer;
    long                read_buffer_size;
    AsioReadcallback    read_callback;
    void*               read_callback_arg;

    int                 write_state;
    void*               write_buffer;
    long                write_buffer_size;
    AsioWritecallback   write_callback;
    void*               write_callback_arg;
    RBL_DECLARE_END_TAG;
} AsioStream, *AsioStreamRef;


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

//struct RunloopInterthreadQueue_s {
//    RBL_DECLARE_TAG;
//    RunloopEventfdRef                      eventfd_ref;
//    ListRef                             queue;
//    pthread_mutex_t                     queue_mutex;
//    InterthreadQueueEventHandler        queue_postable;
//    void*                               queue_postable_arg;
//};


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
#endif