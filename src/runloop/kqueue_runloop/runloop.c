#include <runloop/runloop.h>
#include "rl_internal.h"
#include "event_table.h"
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
#include <common/list.h>

//__thread RunloopRef my_reactor_ptr = NULL;
//
//#define CHECK_THREAD(reactor_ref) //assert(reactor_ref == my_reactor_ptr);



static void drain_callback(void* arg)
{
    printf("drain callback\n");
}
struct kevent* runloop_get_change_table(RunloopRef athis);
int runloop_get_change_table_size(RunloopRef athis);
struct kevent* runloop_change_at(RunloopRef athis, int index);
struct kevent* runloop_get_fresh_event_table(RunloopRef athis);
int runloop_get_max_events(RunloopRef athis);
struct kevent* runloop_events_at(RunloopRef athis, int index);

void runloop_init(RunloopRef rl) {

    RunloopRef runloop = rl;
    RUNLOOP_SET_TAG(runloop)
    RUNLOOP_SET_END_TAG(runloop)
    runloop->kqueue_fd = kqueue();
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    RBL_ASSERT((runloop->kqueue_fd != -1), "kqueue create failed");
    RBL_LOG_FMT("runloop_new kqueue_fd %d", runloop->kqueue_fd);
    runloop->event_table = event_table_new();
    runloop->ready_list = functor_list_new(RL_MAX_RUNLIST);
#if 1
    runloop->change_count = 0;
    runloop->change_max = RL_MAX_EVENTS;
    runloop->events_count = 0;
    runloop->events_max = RL_MAX_EVENTS;
#endif
}
/**
 * Create a new runloop. Should only be one per thread
 * @NOTE - this implementation only works for Linux and uses epoll
 */
RunloopRef runloop_new(void) {
    RunloopRef runloop = malloc(sizeof(Runloop));
    RBL_ASSERT((runloop != NULL), "malloc failed new runloop");
    runloop_init(runloop);
    return (RunloopRef)runloop;
}

void runloop_close(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    athis->closed_flag = true;
    int status = close(athis->kqueue_fd);
    RBL_LOG_FMT("runloop_close status: %d errno: %d", status, errno);
    RBL_ASSERT((status != -1), "close kqueue_fd failed");
}

void runloop_free(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    if(! athis->closed_flag) {
        runloop_close(athis);
    }
    // what to do about event_allocator_free(athis->event_allocator);

    functor_list_free(athis->ready_list);
    free(athis);
}
void print_events(struct kevent events[], int count)
{
    for(int i = 0; i < count; i++) {
        // struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int runloop_run(RunloopRef athis, time_t timeout_ms) {
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    #if 1
//    athis->tid = gettid();
    int result;

    time_t start = time(NULL);

    while (true) {
        RUNLOOP_CHECK_TAG(athis)
        RUNLOOP_CHECK_END_TAG(athis)
        time_t passed = time(NULL) - start;

        printf("runloop functor_list_size: %d event_table_number_in_user %zu \n",
            functor_list_size(athis->ready_list),
            event_table_number_in_use(athis->event_table)
        );
        if(
            ((functor_list_size(athis->ready_list) == 0))
            && (0 == event_table_number_in_use(athis->event_table))
        ) {
            // no more work to do - clean exit
            RBL_LOG_FMT("runloop exiting");
            result = 0;
            goto cleanup;
        }
        int max_events = RL_MAX_EVENTS;
        if(functor_list_size(athis->ready_list) == 0) {
            struct timespec *timeout = NULL;
            struct timespec t = { .tv_sec = timeout_ms / 1000 , .tv_nsec= 1000 *(timeout_ms % 1000)};
            if(timeout_ms > 0) {
                timeout = &t;
            }
            struct  kevent* change = runloop_get_change_table(athis);
            int change_n = runloop_get_change_table_size(athis);
            struct kevent* events = runloop_get_fresh_event_table(athis);
            int max_events = runloop_get_max_events(athis);
            int nev = kevent(athis->kqueue_fd, change, change_n, events, max_events, timeout);
            RBL_LOG_FMT("runloop keventreturned nev: %d fd[0]: %lu events active: %zu  ready_list_size:%d",
                        nev, events[0].ident,
                        event_table_number_in_use(athis->event_table), functor_list_size(athis->ready_list));
            time_t currtime = time(NULL);
            switch (nev) {
                case -1:
                    int saved_errno = errno;
                    if (errno == EINTR) {
                        printf("reactor interrupted\n");
                        result = -1;
                        goto cleanup;
                        continue;
                    } else if (athis->closed_flag) {
                        result = 0;
                    } else {
                        perror("XXX kqueue_wait");
                        result = -1;
                        int ern = errno;
                    }
                    goto cleanup;
                case 0:
                    result = 0;
                    close(athis->kqueue_fd);
                    athis->closed_flag = true;
                    goto cleanup;
                default: {
                    for (int i = 0; i < nev; i++) {
                        RUNLOOP_CHECK_TAG(athis)
                        RUNLOOP_CHECK_END_TAG(athis)
                        struct kevent ke = athis->events[i];
                        void* pp = (void*)ke.ident;
                        RunloopWatcherBaseRef rlwatcher = events[i].udata;

                        int filters = athis->events[i].filter;
                        void* data = (void*)events[i].data;
                        uint32_t flags = athis->events[i].flags;
                        int eof = flags & EV_EOF;
                        RBL_LOG_FMT("runloop_run loop ident: %lu udata: %p events: %x flags: %x eof:%d", ke.ident ,rlevent , filters, flags, eof);
                        rlwatcher->handler(rlwatcher, filters, flags, data);
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
    #endif
    return 0;
}
struct kevent* runloop_get_change_table(RunloopRef athis)
{
    return &(athis->change[0]);
}
int runloop_get_change_table_size(RunloopRef athis)
{
    return athis->change_count;
}
struct kevent* runloop_change_at(RunloopRef athis, int index)
{
    RBL_ASSERT(((index >= 0)&&(index < athis->change_max)), "");
    return &(athis->change[index]);
}
struct kevent* runloop_get_fresh_event_table(RunloopRef athis)
{
    athis->events_count = 0;
    return &(athis->events[0]);
}
int runloop_get_max_events(RunloopRef athis)
{
    return athis->events_max;
}
struct kevent* runloop_events_at(RunloopRef athis, int index)
{
    return &(athis->events[index]);
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
void runloop_verify(RunloopRef rl)
{
    RUNLOOP_CHECK_TAG(rl)
    RUNLOOP_CHECK_END_TAG(rl)
}