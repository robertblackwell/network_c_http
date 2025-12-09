#include <runloop/runloop.h>
#include "runloop_internal.h"
#include <stdint.h>
#include <time.h>
#include <sys/event.h>
#include <sys/time.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <common/object_pool.h>

//__thread RunloopRef my_reactor_ptr = NULL;
//
//#define CHECK_THREAD(reactor_ref) //assert(reactor_ref == my_reactor_ptr);



static void drain_callback(void* arg)
{
    printf("drain callback\n");
}
struct kevent* runloop_get_change_table(RunloopRef runloop_p);
int runloop_get_change_table_size(RunloopRef runloop_p);
struct kevent* runloop_change_at(RunloopRef runloop_p, int index);
struct kevent* runloop_get_fresh_event_table(RunloopRef runloop_p);
int runloop_get_max_events(RunloopRef runloop_p);
struct kevent* runloop_events_at(RunloopRef runloop_p, int index);

struct MemorySlab_s {
        union {
            RunloopTimer     timer;
            RunloopListener  listener;
            RunloopStream    stream;
            RunloopUserEvent user_event;
            RunloopSignal    signal;
        };
};

void* runloop_event_allocate(RunloopRef rl, size_t size)
{
    uint16_t objsize = object_pool_obj_size(rl->object_pool_ref);
    assert(objsize >= size);
    void* p = object_pool_allocate(rl->object_pool_ref);
    size_t ixx = object_pool_number_in_use(rl->object_pool_ref);
    RBL_ASSERT((p != NULL),"runloop failed to allocate event object")
    rl->active_event_count += 1;
    return p;
}

void runloop_event_free(RunloopRef rl, void* p)
{
    RUNLOOP_CHECK_TAG(rl)
    RUNLOOP_CHECK_END_TAG(rl)
    object_pool_deallocate(rl->object_pool_ref, p);
    rl->active_event_count -= 1;
}


void runloop_init(RunloopRef rl, RunloopConfig* config) {

    RunloopRef runloop = rl;
    RUNLOOP_SET_TAG(runloop)
    RUNLOOP_SET_END_TAG(runloop)
    runloop->epoll_kqueue_fd = kqueue();
    runloop->active_event_count = 0;
    runloop->events_count = 0;
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    runloop->max_nbr_events = (config) ? config->max_nbr_events+2: RL_MAX_EVENTS+2;
    runloop->max_simultaneous_callbacks_per_event = (config)
        ? config->max_simultaneous_callbacks_per_event
        : RL_GTHREADS_PER_WATCHER;
    RBL_ASSERT((runloop->epoll_kqueue_fd != -1), "kqueue create failed");
    RBL_LOG_FMT("runloop_new kqueue_fd %d", runloop->kqueue_fd);
    runloop->object_pool_ref = object_pool_create(sizeof(MemorySlab), RL_MAX_EVENTS);
    size_t ixx = object_pool_number_in_use(runloop->object_pool_ref);
    runloop->ready_list = functor_list_new(RL_MAX_RUNLIST);
#if 1
    runloop->change_count = 0;
    runloop->change_max = RL_MAX_EVENTS;
    runloop->events_count = 0;
    runloop->events_max = RL_MAX_EVENTS;
#endif
}
RunloopRef runloop_new_with_config(RunloopConfig* config)
{
    RunloopRef runloop = malloc(sizeof(Runloop));
    RBL_ASSERT((runloop != NULL), "malloc failed new runloop");
    runloop_init(runloop, config);
    return (RunloopRef)runloop;
}
RunloopRef runloop_new(void) {
    RunloopRef runloop = malloc(sizeof(Runloop));
    RBL_ASSERT((runloop != NULL), "malloc failed new runloop");
    runloop_init(runloop, NULL);
    return (RunloopRef)runloop;
}

void runloop_close(RunloopRef runloop_p)
{
    RUNLOOP_CHECK_TAG(runloop_p)
    RUNLOOP_CHECK_END_TAG(runloop_p)
    runloop_p->closed_flag = true;
    int status = close(runloop_p->epoll_kqueue_fd);
    RBL_LOG_FMT("runloop_close status: %d errno: %d", status, errno);
    RBL_ASSERT((status != -1), "close kqueue_fd failed");
}

void runloop_free(RunloopRef runloop_p)
{
    RUNLOOP_CHECK_TAG(runloop_p)
    RUNLOOP_CHECK_END_TAG(runloop_p)
    if(! runloop_p->closed_flag) {
        runloop_close(runloop_p);
    }
    // what to do about event_allocator_free(runloop_p->event_allocator);

    functor_list_free(runloop_p->ready_list);
    // TODO destroy the object pool
    free(runloop_p);
}
void print_events(struct kevent events[], int count)
{
    for(int i = 0; i < count; i++) {
        // struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int runloop_run(RunloopRef runloop_p, time_t timeout_ms) {
    RUNLOOP_CHECK_TAG(runloop_p)
    RUNLOOP_CHECK_END_TAG(runloop_p)
    #if 1
//    runloop_p->tid = gettid();
    int result;

    time_t start = time(NULL);

    while (true) {
        RUNLOOP_CHECK_TAG(runloop_p)
        RUNLOOP_CHECK_END_TAG(runloop_p)
        time_t passed = time(NULL) - start;

        RBL_LOG_FMT("runloop functor_list_size: %d event_table_number_in_user %zu",
            functor_list_size(runloop_p->ready_list),
            object_pool_number_in_use(runloop_p->object_pool_ref)
        );
        size_t ixx = object_pool_number_in_use(runloop_p->object_pool_ref);
        if(object_pool_number_in_use(runloop_p->object_pool_ref) != runloop_p->active_event_count) {
            RBL_ASSERT((object_pool_number_in_use(runloop_p->object_pool_ref) == runloop_p->active_event_count), "")
        }
        if(
            ((functor_list_size(runloop_p->ready_list) == 0))
            &&(runloop_p->active_event_count == 0)
            // && (0 == object_pool_number_in_use(runloop_p->object_pool_ref))
        ) {
            // no more work to do - clean exit
            RBL_LOG_FMT("runloop exiting");
            result = 0;
            goto cleanup;
        }
        if(functor_list_size(runloop_p->ready_list) == 0) {
            struct timespec *timeout = NULL;
            struct timespec t = { .tv_sec = timeout_ms / 1000 , .tv_nsec= 1000 *(timeout_ms % 1000)};
            if(timeout_ms > 0) {
                timeout = &t;
            }
            struct  kevent* change = runloop_get_change_table(runloop_p);
            int change_n = runloop_get_change_table_size(runloop_p);
            struct kevent* events = runloop_get_fresh_event_table(runloop_p);
            int max_events = runloop_get_max_events(runloop_p);
            int nev = kevent(runloop_p->epoll_kqueue_fd, change, change_n, events, max_events, timeout);
            RBL_LOG_FMT("runloop keventreturned nev: %d fd[0]: %lu events active: %zu  ready_list_size:%d",
                        nev, events[0].ident,
                        event_table_number_in_use(runloop_p->event_table), functor_list_size(runloop_p->ready_list));
            time_t currtime = time(NULL);
            switch (nev) {
                case -1:
                    int saved_errno = errno;
                    if (errno == EINTR) {
                        printf("reactor interrupted\n");
                        result = -1;
                        goto cleanup;
                        continue;
                    } else if (runloop_p->closed_flag) {
                        result = 0;
                    } else {
                        perror("XXX kqueue_wait");
                        result = -1;
                        int ern = errno;
                    }
                    goto cleanup;
                case 0:
                    result = 0;
                    close(runloop_p->epoll_kqueue_fd);
                    runloop_p->closed_flag = true;
                    goto cleanup;
                default: {
                    for (int i = 0; i < nev; i++) {
                        RUNLOOP_CHECK_TAG(runloop_p)
                        RUNLOOP_CHECK_END_TAG(runloop_p)
                        struct kevent ke = runloop_p->events[i];
                        void* pp = (void*)ke.ident;
                        RunloopEventBaseRef rlwatcher = events[i].udata;

                        int filters = runloop_p->events[i].filter;
                        void* data = (void*)events[i].data;
                        uint32_t flags = runloop_p->events[i].flags;
                        int eof = flags & EV_EOF;
                        RBL_LOG_FMT("runloop_run loop ident: %lu udata: %p events: %x flags: %x eof:%d", ke.ident ,rlevent , filters, flags, eof);
                        rlwatcher->handler(rlwatcher, filters, flags, data);
                        RUNLOOP_CHECK_TAG(runloop_p)
                    }
                }
            }
        } else {
            FunctorRef fnc;
            while (1) {
                RUNLOOP_CHECK_TAG(runloop_p)
                RUNLOOP_CHECK_END_TAG(runloop_p)
                if (functor_list_size(runloop_p->ready_list) == 0) {
                    break;
                }
                Functor func = functor_list_remove(runloop_p->ready_list);
                runloop_p->runloop_executing = true;
                func.f(runloop_p, func.arg);
                runloop_p->runloop_executing = false;
                RUNLOOP_CHECK_TAG(runloop_p)
                if (functor_list_size(runloop_p->ready_list) == 0) {
                    RBL_LOG_FMT("reactor runlist loop  break functor_list_size: %d func: %p arg: %p",
                                functor_list_size(runloop_p->ready_list), func.f, func.arg);
                    break;
                }
            }
        }
    }

cleanup:
    return result;
    #endif
    return 0;
}
struct kevent* runloop_get_change_table(RunloopRef runloop_p)
{
    return &(runloop_p->change[0]);
}
int runloop_get_change_table_size(RunloopRef runloop_p)
{
    return runloop_p->change_count;
}
struct kevent* runloop_change_at(RunloopRef runloop_p, int index)
{
    RBL_ASSERT(((index >= 0)&&(index < runloop_p->change_max)), "");
    return &(runloop_p->change[index]);
}
struct kevent* runloop_get_fresh_event_table(RunloopRef runloop_p)
{
    runloop_p->events_count = 0;
    return &(runloop_p->events[0]);
}
int runloop_get_max_events(RunloopRef runloop_p)
{
    return runloop_p->events_max;
}
struct kevent* runloop_events_at(RunloopRef runloop_p, int index)
{
    return &(runloop_p->events[index]);
}

void runloop_post(RunloopRef runloop_p, PostableFunction cb, void* arg)
{
    RUNLOOP_CHECK_TAG(runloop_p)
    RUNLOOP_CHECK_END_TAG(runloop_p)
//    assert(runloop_p->tid == gettid());
    RBL_LOG_FMT("runloop_post entered functor_list_size: %d funct: %p arg: %p runloop_executing: %d", functor_list_size(runloop_p->ready_list), cb, arg, (int)runloop_p->runloop_executing);
    assert(cb != NULL);
    assert(arg != NULL);
    Functor func = {.f = cb, .arg = arg};
    functor_list_add(runloop_p->ready_list, func);
    RBL_LOG_FMT("runloop_post exited functor_list_size: %d func: %p arg: %p runloop_executing: %d", functor_list_size(runloop_p->ready_list), cb, arg, (int)runloop_p->runloop_executing);
}
void runloop_verify(RunloopRef rl)
{
    RUNLOOP_CHECK_TAG(rl)
    RUNLOOP_CHECK_END_TAG(rl)
}