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

struct kevent* runloop_get_change_table(RunloopRef runloop_p);
int runloop_get_change_table_size(RunloopRef runloop_p);
struct kevent* runloop_change_at(RunloopRef runloop_p, int index);
struct kevent* runloop_get_fresh_event_table(RunloopRef runloop_p);
int runloop_get_max_events(RunloopRef runloop_p);
struct kevent* runloop_events_at(RunloopRef runloop_p, int index);

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
