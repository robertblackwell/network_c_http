#ifndef C_HTTP_RL_INTERNAL_H
#define C_HTTP_RL_INTERNAL_H
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_checktag.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <http_in_c/common/list.h>

#define RTOR_MAX_FDS            1024
#define RTOR_MAX_RUNLIST        1024
#define RTOR_MAX_ITQ            256
#define RTOR_MAX_WATCHERS       RTOR_MAX_FDS
#define RTOR_FUNCTOR_LIST_MAX   RTOR_MAX_ITQ
#define RTOR_MAX_EPOLL_FDS      RTOR_MAX_FDS
#define CBTABLE_MAX             RTOR_MAX_FDS
#define RTOR_FDTABLE_MAX        RTOR_MAX_FDS
#define RTOR_READY_LIST_MAX     (2 * RTOR_MAX_FDS)

// enables use of eventfd rather than two pipe trick
#define  RTOR_EVENTFD_ENABLE


struct FdTable_s;
//===============
#define CBTABLE_MAX 4096

typedef struct FdTable_s FdTable, *FdTableRef;
FdTableRef FdTable_new();
void       FdTable_free(FdTableRef athis);
void       FdTable_insert(FdTableRef athis, RtorWatcherRef wref, int fd);
void       FdTable_remove(FdTableRef athis, int fd);
RtorWatcherRef FdTable_lookup(FdTableRef athis, int fd);
int        FdTable_iterator(FdTableRef athis);
int        FdTable_next_iterator(FdTableRef athis, int iter);
uint64_t   FdTable_size(FdTableRef athis);

typedef ListRef RunListRef;
typedef ListIter RunListIter;

// A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_init(FunctorRef funref, PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis, ReactorRef rtor_ref);
void Functor_dealloc(void **p);
struct Functor_s
{
//    RtorWatcherRef wref; // this is borrowed do not free
    PostableFunction f;
    void *arg;
};

/**
 * runlist - is a list of Functors - these are functions that are ready to run.
 * Use should be confined to a single thread - no synchronization.
 * Intended to be used within a Reactor or Runloop
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
    char       tag[TAG_LENGTH];
    int        capacity;
    int        head;
    int        tail_plus;
    FunctorRef list; // points to an array of Functor objects
    char       end_tag[TAG_LENGTH];
} FunctorList, *FunctorListRef;

/**
 * NOTE: FunctionList acceptt and return values of a Functor NOT a pointer
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

struct Reactor_s {
    DECLARE_TAG;
    int                     epoll_fd;
    bool                    closed_flag;
    FdTableRef              table; // (int, CallbackData)
    FunctorListRef          ready_list;
#if 1
    EvfdQueueRef            interthread_queue_ref;
    RtorWQueueRef           interthread_queue_watcher_ref;
#else
    RtorInterthreadQueueRef interthread_queue;
#endif
};
/**
 * RtorWatcher - a generic observer object
 */
typedef enum WatcherType {
    XR_WATCHER_SOCKET = 11,
    XR_WATCHER_TIMER = 12,
    XR_WATCHER_QUEUE = 13,
    XR_WATCHER_FDEVENT = 14,
    XR_WATCHER_LISTENER = 15,
} WatcherType;


struct RtorWatcher_s {
    DECLARE_TAG;
    WatcherType           type;
    ReactorRef            runloop;
    void*                 context;
    int                   fd;
    /**
     * function that knows how to free the specific sub type of watcher from a general ref.
     * each derived type must provide this function when an instance is created or initializez.
     * In the case of timerfd and event fd watchers must also close the fd
     */
    void(*free)(RtorWatcherRef);
    /**
     * first level handler function
     * each derived type provides thier own type specific handler when an instance is created
     * or initialized and must cast the first parameter to their own specific type of watcher
     * inside the handler.
     * 
     * This handler will be calledd directly from the epoll_wait code inside reactor.c
    */
    void(*handler)(RtorWatcherRef watcher_ref, uint64_t event);
};


typedef struct EvfdQueue_s {
    FunctorListRef      list;
    pthread_mutex_t     queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int                 pipefds[2];
#endif
    int                 readfd;
    int                 writefd;
    int                 id;
} EvfdQueue;

typedef uint64_t WEventFdMask;
struct RtorEventfd_s {
    struct RtorWatcher_s;
    FdEventHandler      fd_event_handler;
    void*               fd_event_handler_arg;
    int                 write_fd;
};

/**
 * RtorStream
 */
struct RtorStream_s {
    struct RtorWatcher_s;
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
typedef struct RtorListener_s {
    struct RtorWatcher_s;
    ListenerEventHandler     listen_evhandler;
    void*                    listen_arg;
} RtorListener;

/**
 * RtorWQueue
 */
typedef uint64_t XrQueueEvent;
typedef void(XrQueuetWatcherCaller(void* ctx));
struct RtorWQueue_s {
    struct RtorWatcher_s;
    EvfdQueueRef            queue;
    // reactor cb and arg
    QueueEventHandler       queue_event_handler;
    void*                   queue_event_handler_arg;
};

/**
 * InterThreadQueue
 */
typedef uint64_t XrITQueueEvent;
typedef void(XrITQueuetWatcherCaller(void* ctx));
struct RtorInterthreadQueue_s {
    DECLARE_TAG;
    RtorEventfdRef                      eventfd_ref;
    ListRef                             queue;
    pthread_mutex_t                     queue_mutex;
    InterthreadQueueEventHandler        queue_event_handler;
    void*                               queue_event_handler_arg;
};


/**
 * RtorTimer
 */
typedef uint64_t XrTimerEvent;
struct RtorTimer_s {
    struct RtorWatcher_s;
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    TimerEventHandler       timer_handler;
    void*                   timer_handler_arg;
};

#endif