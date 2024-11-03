#ifndef C_HTTP_RL_INTERNAL_H
#define C_HTTP_RL_INTERNAL_H
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_checktag.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <http_in_c/common/list.h>

#define runloop_MAX_FDS            1024
#define runloop_MAX_RUNLIST        1024
#define runloop_MAX_ITQ            256
#define runloop_MAX_WATCHERS       runloop_MAX_FDS
#define runloop_FUNCTOR_LIST_MAX   runloop_MAX_ITQ
#define runloop_MAX_EPOLL_FDS      runloop_MAX_FDS
#define CBTABLE_MAX             runloop_MAX_FDS
#define runloop_FDTABLE_MAX        runloop_MAX_FDS
#define runloop_READY_LIST_MAX     (2 * runloop_MAX_FDS)

// enables use of eventfd rather than two pipe trick
#define  runloop_eventfd_ENABLE


struct FdTable_s;
//===============
#define CBTABLE_MAX 4096

typedef struct FdTable_s FdTable, *FdTableRef;
FdTableRef FdTable_new();
void       FdTable_free(FdTableRef athis);
void       FdTable_insert(FdTableRef athis, RunloopWatcherRef wref, int fd);
void       FdTable_remove(FdTableRef athis, int fd);
RunloopWatcherRef FdTable_lookup(FdTableRef athis, int fd);
int        FdTable_iterator(FdTableRef athis);
int        FdTable_next_iterator(FdTableRef athis, int iter);
uint64_t   FdTable_size(FdTableRef athis);

typedef ListRef RunListRef;
typedef ListIter RunListIter;

/**
 * A Functor is a generic callback - a function pointer (of type PostableFunction) and single anonymous argument.
 *
 * The significant thing is that the function pointer, points to a function that has the correct
 * signature for the RunList
 *
*/
 struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_init(FunctorRef funref, PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis, RunloopRef runloop_ref);
bool Functor_is_empty(FunctorRef f);
void Functor_dealloc(void **p);
struct Functor_s
{
//    RunloopWatcherRef wref; // this is borrowed do not free
    PostableFunction f;
    void *arg;
};

/**
 * runlist - is a list of Functors - these are functors that are ready to run.
 * Use should be confined to a single thread as there is no synchronization.
 * Intended to be used within a Runloop or Runloop
 */
RunListRef RunList_new();
//======================

void RunList_dispose();
void RunList_add_back(RunListRef this, FunctorRef f);
FunctorRef RunList_remove_front(RunListRef this);
int RunList_size (RunListRef rl_ref);
FunctorRef  RunList_first (RunListRef rl_ref);
FunctorRef  RunList_last (RunListRef rl_ref);
FunctorRef  RunList_remove_first (RunListRef rl_ref);
FunctorRef  RunList_remove_last (RunListRef rl_ref);
FunctorRef  RunList_itr_unpack (RunListRef rl_ref, RunListIter iter);
RunListIter RunList_iterator (RunListRef rl_ref);
RunListIter RunList_itr_next (RunListRef rl_ref, RunListIter iter);
void RunList_itr_remove (RunListRef rl_ref, RunListIter *iter);

typedef struct InterthreadRunList_s InterthreadRunList, *InterthreadRunListRef;

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
    FdTableRef              table; // (int, CallbackData)
    FunctorListRef          ready_list;
#if 1
    EventfdQueueRef         interthread_queue_ref;
    RunloopQueueWatcherRef  interthread_queue_watcher_ref;
#else
    RunloopInterthreadQueueRef interthread_queue;
#endif
    RBL_DECLARE_END_TAG;
};
/**
 * RunloopWatcher - a generic observer object
 */
typedef enum WatcherType {
    RUNLOOP_WATCHER_SOCKET = 11,
    RUNLOOP_WATCHER_TIMER = 12,
    RUNLOOP_WATCHER_QUEUE = 13,
    RUNLOOP_WATCHER_FDEVENT = 14,
    RUNLOOP_WATCHER_LISTENER = 15,
} WatcherType;


struct RunloopWatcher_s {
    RBL_DECLARE_TAG;
    WatcherType           type;
    RunloopRef            runloop;
    void*                 context;
    int                   fd;
    /**
     * function that knows how to free the specific sub type of watcher from a general ref.
     * each derived type must provide this function when an instance is created or initializez.
     * In the case of timerfd and event fd watchers must also close the fd
     */
    void(*free)(RunloopWatcherRef);
    /**
     * first level handler function
     * each derived type provides thier own type specific handler when an instance is created
     * or initialized and must cast the first parameter to their own specific type of watcher
     * inside the handler.
     * 
     * This handler will be calledd directly from the epoll_wait code inside runloop.c
    */
    void(*handler)(RunloopWatcherRef watcher_ref, uint64_t event);
    RBL_DECLARE_END_TAG;
};


typedef struct EventfdQueue_s {
    FunctorListRef      list;
    pthread_mutex_t     queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int                 pipefds[2];
#endif
    int                 readfd;
    int                 writefd;
    int                 id;
} EventfdQueue;

typedef uint64_t WEventFdMask;
struct RunloopEventfd_s {
    struct RunloopWatcher_s;
    FdEventHandler      fd_event_handler;
    void*               fd_event_handler_arg;
    int                 write_fd;
};

/**
 * RunloopStream
 */
struct RunloopStream_s {
    struct RunloopWatcher_s;
    uint64_t                 event_mask;
    SocketEventHandler       both_handler;
    void*                    both_arg;
    SocketEventHandler       read_evhandler;
    void*                    read_arg;
    SocketEventHandler       write_evhandler;
    void*                    write_arg;
};

/**
 * WListener
 */
typedef struct RunloopListener_s {
    struct RunloopWatcher_s;
    ListenerEventHandler     listen_evhandler;
    void*                    listen_arg;
} RunloopListener;

/**
 * RunloopQueueWatcher
 */
typedef uint64_t RunloopQueueEvent;
typedef void(RunloopQueuetWatcherCallerback(void* ctx));
struct RunloopQueueWatcher_s {
    struct RunloopWatcher_s;
    EventfdQueueRef            queue;
    // reactor cb and arg
    QueueEventHandler       queue_event_handler;
    void*                   queue_event_handler_arg;
};

/**
 * InterThreadQueue
 */
typedef uint64_t RunloopInterthreadQueueEvent;
typedef void(RunloopInterthreadQueuetWatcherCallerback(void* ctx));
struct InterthreadQueue_s {
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
//    InterthreadQueueEventHandler        queue_event_handler;
//    void*                               queue_event_handler_arg;
//};


/**
 * RunloopTimer
 */
typedef uint64_t RunloopTimerEvent;
struct RunloopTimer_s {
    struct RunloopWatcher_s;
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    PostableFunction        timer_postable;
    void*                   timer_postable_arg;
};

#endif