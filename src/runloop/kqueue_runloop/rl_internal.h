#ifndef C_HTTP_KQRL_INTERNAL_H
#define C_HTTP_KQRL_INTERNAL_H
#include <runloop/kqueue_runloop/runloop.h>
#include <runloop/kqueue_runloop/rl_checktag.h>
#include <sys/event.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <common/list.h>

#define runloop_MAX_FDS            1024
#define runloop_MAX_RUNLIST        1024
#define runloop_MAX_ITQ            256
#define runloop_MAX_WATCHERS       runloop_MAX_FDS
#define runloop_FUNCTOR_LIST_MAX   runloop_MAX_ITQ
#define runloop_MAX_KQ_FDS         runloop_MAX_FDS
#define runloop_MAX_EVENTS         runloop_MAX_FDS*2
#define CBTABLE_MAX                runloop_MAX_FDS
#define runloop_FDTABLE_MAX        runloop_MAX_FDS
#define runloop_READY_LIST_MAX     (2 * runloop_MAX_FDS)

// enables use of eventfd rather than two pipe trick
#define  runloop_eventfd_ENABLE


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

// /**
//  * A Functor is a generic callback - a function pointer (of type PostableFunction) and single anonymous argument.
//  *
//  * The significant thing is that the function pointer, points to a function that has the correct
//  * signature for the RunList
//  * TYpes defined in runloop.h
// */
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
    FunctorRef list; // points to an array of Functor objects - size determined at runtime
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
    int                     kqueue_fd;
    bool                    closed_flag;
    bool                    runloop_executing;
    pthread_t               tid;
    FdTableRef              table; // (int, CallbackData)
    void*                   event_allocator;
    struct kevent           change[runloop_MAX_EVENTS];
    int                     change_max;
    int                     change_count;
    struct kevent           events[runloop_MAX_EVENTS];
    int                     events_max;
    int                     events_count;
    FunctorListRef          ready_list;
    RBL_DECLARE_END_TAG;
};

#include "rl_events_internal.h"
#include <kqueue_runloop/event_allocator.h>

#endif