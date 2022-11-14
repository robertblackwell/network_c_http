#ifndef rl_internal_h
#define rl_internal_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_checktag.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <c_http/common/list.h>


struct FdTable_s;
//===============
#define CBTABLE_MAX 4096

typedef struct FdTable_s FdTable, *FdTableRef;
FdTableRef FdTable_new();
void       FdTable_free(FdTableRef athis);
void       FdTable_insert(FdTableRef athis, WatcherRef wref, int fd);
void       FdTable_remove(FdTableRef athis, int fd);
WatcherRef FdTable_lookup(FdTableRef athis,int fd);
int        FdTable_iterator(FdTableRef athis);
int        FdTable_next_iterator(FdTableRef athis, int iter);
uint64_t   FdTable_size(FdTableRef athis);

typedef ListRef RunListRef;
typedef ListIter RunListIter;
// A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis);

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

void fd_map_init();
bool fd_map_at(int j);
bool fd_map_set(int j);

/**
 * Scan down the run list calling each Functor entry until the list is empty;
 * @param this
 */
void RunList_exec(RunListRef this);

struct Reactor_s {
    XR_REACTOR_DECLARE_TAG;
    int               epoll_fd;
    bool              closed_flag;
    FdTableRef        table; // (int, CallbackData)
    RunListRef        run_list;
};
/**
 * Watcher - a generic observer object
 */
typedef enum WatcherType {
    XR_WATCHER_SOCKET = 11,
    XR_WATCHER_TIMER = 12,
    XR_WATCHER_QUEUE = 13,
    XR_WATCHER_FDEVENT = 14,
    XR_WATCHER_LISTENER = 15,
} WatcherType;


struct Watcher_s {
    XR_WATCHER_DECLARE_TAG;
    WatcherType           type;
    ReactorRef            runloop;
    int                   fd;
    /**
     * function that knows how to free the specific sub type of watcher from a general ref.
     * each derived type must provide this function when an instance is created or initializez.
     * In the case of timerfd and event fd watchers must also close the fd
     */
    void(*free)(WatcherRef);
    /**
     * first level handler function
     * each derived type provides thier own type specific handler when an instance is created
     * or initialized and must cast the first parameter to their own specific type of watcher
     * inside the handler.
     * 
     * This handler will be calledd directly from the epoll_wait code inside reactor.c
    */
    void(*handler)(WatcherRef watcher_ref, int fd, uint64_t event);
};


#define  C_HTTP_EFD_QUEUE
typedef struct EvfdQueue_s {
    ListRef         list;
    pthread_mutex_t queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int             pipefds[2];
#endif
    int             readfd;
    int             writefd;
    int             id;
} EvfdQueue;

typedef uint64_t WEventFdMask;
struct WEventFd_s {
    struct Watcher_s;
    FdEventHandler      fd_event_handler;
    void*               fd_event_handler_arg;
    int                 write_fd;
};

/**
 * WIoFd
 */
struct WIoFd_s {
    struct Watcher_s;

    uint64_t                 event_mask;
    SocketEventHandler       read_evhandler;
    void*                    read_arg;
    SocketEventHandler       write_evhandler;
    void*                    write_arg;
};

/**
 * WListener
 */
typedef struct WListenerFd_s {
    struct Watcher_s;
    ListenerEventHandler     listen_evhandler;
    void*                    listen_arg;
} WListenerFd;

/**
 * WQueue
 */
typedef uint64_t XrQueueEvent;
typedef void(XrQueuetWatcherCaller(void* ctx));
struct WQueue_s {
    struct Watcher_s;
    EvfdQueueRef            queue;
    // reactor cb and arg
    QueueEventHandler       queue_event_handler;
    void*                   queue_event_handler_arg;
};

/**
 * WTimerFd
 */
typedef uint64_t XrTimerEvent;
struct WTimerFd_s {
    struct Watcher_s;
    /**
     * XrTimerWatecher specific properties
     */
    time_t                  expiry_time;
    uint64_t                interval;
    bool                    repeating;
    TimerEventHandler       timer_handler;
    void*                   timer_handler_arg;
};

#endif