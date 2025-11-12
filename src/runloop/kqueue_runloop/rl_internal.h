#ifndef C_HTTP_KQRL_INTERNAL_H
#define C_HTTP_KQRL_INTERNAL_H
#include <runloop/runloop.h>
#include <runloop/functor.h>
#include <sys/event.h>
#include <pthread.h>
#include <stdbool.h>
#include <common/object_pool.h>

typedef struct EventTable_s EventTable, *EventTableRef;

#define REGISTER_WQUEUE_REACTOR 1

struct Runloop_s {
    RBL_DECLARE_TAG;
    int                     kqueue_fd;
    bool                    closed_flag;
    bool                    runloop_executing;
    pthread_t               tid;
    ObjectPoolRef           object_pool_ref;
#if 1
    struct kevent           change[RL_MAX_EVENTS];
    int                     change_max;
    int                     change_count;
    struct kevent           events[RL_MAX_EVENTS];
    int                     events_max;
    int                     events_count;
#endif
    FunctorListRef          ready_list;
    RBL_DECLARE_END_TAG;
};


#endif
