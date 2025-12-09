#ifndef C_HTTP_RL_INTERNAL_H
#define C_HTTP_RL_INTERNAL_H
#include <runloop/runloop.h>
#include <runloop/functor.h>
#if defined(APPLE_FLAG)
#include <sys/event.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <common/object_pool.h>

typedef struct EventTable_s EventTable, *EventTableRef;

#define REGISTER_WQUEUE_REACTOR 1

struct Runloop_s {
    RBL_DECLARE_TAG;
    int                     epoll_kqueue_fd;
    #
    bool                    closed_flag;
    bool                    runloop_executing;
    pthread_t               tid;
    ObjectPoolRef           object_pool_ref;
    size_t                  active_event_count;
    FunctorListRef          ready_list;
    int                     max_nbr_events;
    int                     max_simultaneous_callbacks_per_event;
#if defined(APPLE_FLAG)
    struct kevent           change[RL_MAX_EVENTS];
    int                     change_max;
    int                     change_count;
    struct kevent           events[RL_MAX_EVENTS];
    int                     events_max;
    int                     events_count;
#endif
    RBL_DECLARE_END_TAG;
};


#endif
