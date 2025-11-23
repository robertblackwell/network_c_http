#include "runloop_internal.h"
#include <stdint.h>
#include <time.h>
#include <sys/epoll.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <common/object_pool.h>

typedef union Mslab_u {
    RunloopTimer       timer;
    RunloopListener    listener;
    RunloopStream      stream;
    RunloopUserEvent   user_event;
    // RunloopQueueEvent   qevent;
    // RunloopInterthreadQueueEvent itqevent;
    // RunloopQueueWatcher qwatcher;
    // RunloopEvent        runloop_event;
} Mslab;


void* rl_event_allocate(RunloopRef rl, size_t size)
{
    uint16_t objsize = object_pool_obj_size(rl->object_pool_ref);
    assert(objsize >= size);
    void* p = object_pool_allocate(rl->object_pool_ref);
    return p;
}

void rl_event_free(RunloopRef rl, void* p)
{
    object_pool_deallocate(rl->object_pool_ref, p);
}
/**
 * Create a new runloop. Should only be one per thread
 * @NOTE - this implementation only works for Linux and uses epoll
 */
void runloop_init(RunloopRef athis, RunloopConfig* config) {

    RunloopRef runloop = athis;
    RUNLOOP_SET_TAG(runloop)
    RUNLOOP_SET_END_TAG(runloop)
    runloop->epoll_fd = epoll_create1(0);
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    runloop->max_nbr_events = (config) ? config->max_nbr_events+2: RL_MAX_EVENTS+2;
    runloop->max_simultaneous_callbacks_per_event = (config)
        ? config->max_simultaneous_callbacks_per_event
        : RL_GTHREADS_PER_WATCHER;
    RBL_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    RBL_LOG_FMT("runloop_new epoll_fd %d", runloop->epoll_fd);
    runloop->object_pool_ref = object_pool_create(sizeof(Mslab), runloop->max_nbr_events);
    runloop->ready_list = functor_list_new(runloop->max_nbr_events * runloop->max_simultaneous_callbacks_per_event );
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

void runloop_close(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    athis->closed_flag = true;
    int status = close(athis->epoll_fd);
    RBL_LOG_FMT("runloop_close status: %d errno: %d", status, errno);
    RBL_ASSERT((status != -1), "close epoll_fd failed");
}

void runloop_free(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    if(! athis->closed_flag) {
        runloop_close(athis);
    }
    object_pool_destroy(athis->object_pool_ref);
    functor_list_free(athis->ready_list);
    free(athis);
}

void runloop_delete(RunloopRef athis, int fd)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
}
void print_events(struct epoll_event events[], int count)
{
    for(int i = 0; i < count; i++) {
        struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int runloop_run(RunloopRef athis, long timeout_milli_secs) {
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
//    athis->tid = gettid();
    int result;
    struct epoll_event events[RL_MAX_EVENTS];

    time_t start = time(NULL);

    while (true) {
        RUNLOOP_CHECK_TAG(athis)
        RUNLOOP_CHECK_END_TAG(athis)
        time_t passed = time(NULL) - start;

        RBL_LOG_FMT("runloop functor_list_size: %d event_table_number_in_user %zu",
               functor_list_size(athis->ready_list),
               event_table_number_in_use(athis->event_table_ref)
        );
        if(
            ((functor_list_size(athis->ready_list) == 0))
            &&(0 == object_pool_number_in_use(athis->object_pool_ref))
        ) {
            // no more work to do - clean exit
            result = 0;
            goto cleanup;
        }
        int int_timeout_milli_secs = (int)timeout_milli_secs;
        int max_events = RL_MAX_EVENTS;
        if(functor_list_size(athis->ready_list) == 0) {
            int nfds = epoll_wait(athis->epoll_fd, events, max_events, int_timeout_milli_secs);
            time_t currtime = time(NULL);
            switch (nfds) {
                case -1:
                    int saved_errno = errno;
                    if (errno == EINTR) {
                        printf("runloop interrupted\n");
                        result = -1;
                        goto cleanup;
                        continue;
                    } else if (athis->closed_flag) {
                        result = 0;
                    } else {
                        perror("XXX epoll_wait");
                        result = -1;
                        int ern = errno;
                    }
                    goto cleanup;
                case 0:
                    result = 0;
                    close(athis->epoll_fd);
                    athis->closed_flag = true;
                    goto cleanup;
                default: {
                    for (int i = 0; i < nfds; i++) {
                        RUNLOOP_CHECK_TAG(athis)
                        RUNLOOP_CHECK_END_TAG(athis)
#if 1
                        RunloopWatcherBaseRef wr = events[i].data.ptr;
                        // here check we got a valid event watcher
                        int fd = wr->fd;
#else
                        int fd = events[i].data.fd;
                        void *arg = events[i].data.ptr;
#endif
                        int mask = events[i].events;
                        wr->handler(wr, events[i].events);
                        // call handler
                        RUNLOOP_CHECK_TAG(athis)
                    }
                }
            }
        } else {
            FunctorRef fnc;
            while (1) {
                RUNLOOP_CHECK_TAG(athis)
                RUNLOOP_CHECK_END_TAG(athis)
                if (functor_list_size(athis->ready_list) == 0) {
                    break;
                }
                Functor func = functor_list_remove(athis->ready_list);
                athis->runloop_executing = true;
                func.f(athis, func.arg);
                athis->runloop_executing = false;
                RUNLOOP_CHECK_TAG(athis)
                if (functor_list_size(athis->ready_list) == 0) {
                    RBL_LOG_FMT("reactor runlist loop  break functor_list_size: %d func: %p arg: %p",
                                functor_list_size(athis->ready_list), func.f, func.arg);
                    break;
                }
            }
        }
    }

cleanup:
    return result;
}

void runloop_post(RunloopRef athis, PostableFunction cb, void* arg)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
//    assert(athis->tid == gettid());
    RBL_LOG_FMT("runloop_post entered functor_list_size: %d funct: %p arg: %p runloop_executing: %d", functor_list_size(athis->ready_list), cb, arg, (int)athis->runloop_executing);
    assert(cb != NULL);
    assert(arg != NULL);
    Functor func = {.f = cb, .arg = arg};
    functor_list_add(athis->ready_list, func);
    RBL_LOG_FMT("runloop_post exited functor_list_size: %d func: %p arg: %p runloop_executing: %d", functor_list_size(athis->ready_list), cb, arg, (int)athis->runloop_executing);
}
