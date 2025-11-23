#ifndef C_HTTP_epoll_RL_INTERNAL_H
#define C_HTTP_epoll_RL_INTERNAL_H
#include <runloop/runloop.h>
#include <runloop/functor.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <common/list.h>
typedef void* EventTableRef;

struct Runloop_s {
    RBL_DECLARE_TAG;
    int                     epoll_fd;
    bool                    closed_flag;
    bool                    runloop_executing;
    pid_t                   tid;
    ObjectPoolRef           object_pool_ref;
    FunctorListRef          ready_list;
    int                     max_nbr_events;
    int                     max_simultaneous_callbacks_per_event;
    RBL_DECLARE_END_TAG;
};
#endif