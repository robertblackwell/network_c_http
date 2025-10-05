#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/rl_internal.h>
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
static void interthread_queue_handler(RunloopQueueWatcherRef watcher, uint64_t event)
{
    printf("interthread_queue_handler\n");
    return;
    RunloopRef rx = runloop_queue_watcher_get_reactor(watcher);
    EventfdQueueRef evqref = watcher->queue;
    Functor func = runloop_eventfd_queue_remove(evqref);
    void* pf = func.f;
    watcher->queue_postable_arg = func.arg;
    void* arg = (void*) watcher;
    long d = (long) func.arg;
    printf("reactor::interthread_queue_handler f: %p d: %ld \n", pf, d);
    runloop_post(rx, func.f, arg);
}
static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}
struct kevent* runloop_get_change_table(RunloopRef athis);
int runloop_get_change_table_size(RunloopRef athis);
struct kevent* runloop_change_at(RunloopRef athis, int index);
struct kevent* runloop_get_fresh_event_table(RunloopRef athis);
int runloop_get_max_events(RunloopRef athis);
struct kevent* runloop_events_at(RunloopRef athis, int index);

/**
 * Perform a general epoll_ctl call with error checking.
 * In the event of an error abort 
 */
static void runloop_kevent(RunloopRef athis, int op, int fd, uint64_t interest, void* watcher)
{
    RUNLOOP_CHECK_TAG(athis)
    // struct epoll_event epev = {
    //     .events = interest,
    //     .data = {
    //         .fd = fd,
    //     }
    // };
    // // note epev.data is a union so the next line overites .fd = fd
    // epev.data.ptr = watcher;
    // int status = epoll_ctl(athis->epoll_fd, op, fd, &(epev));
    // if (status != 0) {
    //     int errno_saved = errno;
    //     RBL_LOG_ERROR("runloop_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", athis->epoll_fd, fd, status, errno_saved, strerror(errno_saved));
    // }
    RBL_LOG_FMT("runloop_epoll_ctl epoll_fd: %d status : %d errno : %d", athis->epoll_fd, status, errno);
    // RBL_ASSERT((status == 0), "epoll ctl call failed");
}

//RunloopRef runloop_get_threads_reactor()
//{
//    return my_reactor_ptr;
//}

/**
 * Create a new reactor runloop. Should only be one per thread
 * @NOTE - this implementation only works for Linux and uses epoll
 */
RunloopRef runloop_new(void) {
    RunloopRef runloop = malloc(sizeof(Runloop));
    RBL_ASSERT((runloop != NULL), "malloc failed new runloop");
    runloop_init(runloop);
    return (RunloopRef)runloop;
}
void runloop_init(RunloopRef athis) {

    RunloopRef runloop = athis;

    RUNLOOP_SET_TAG(runloop)
    RUNLOOP_SET_END_TAG(runloop)
//    runloop->tid = gettid();
    runloop->kqueue_fd = kqueue();
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    RBL_ASSERT((runloop->kqueue_fd != -1), "kqueue create failed");
    RBL_LOG_FMT("runloop_new kqueue_fd %d", runloop->kqueue_fd);
    runloop->table = FdTable_new();
    runloop->event_allocator = event_allocator_new();
    runloop->ready_list = functor_list_new(runloop_READY_LIST_MAX);
    runloop->change_count = 0;
    runloop->change_max = runloop_MAX_EVENTS;
    runloop->events_count = 0;
    runloop->events_max = runloop_MAX_EVENTS;
}
void runloop_close(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    athis->closed_flag = true;
    int status = close(athis->kqueue_fd);
    RBL_LOG_FMT("runloop_close status: %d errno: %d", status, errno);
    RBL_ASSERT((status != -1), "close epoll_fd failed");
    int next_fd = FdTable_iterator(athis->table);
    while (next_fd  != -1) {
        close(next_fd);
        next_fd = FdTable_next_iterator(athis->table, next_fd);
    }
}

void runloop_free(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    if(! athis->closed_flag) {
        runloop_close(athis);
    }
    FdTable_free(athis->table);
    functor_list_free(athis->ready_list);
    free(athis);
}
int runloop_register_timer(RunloopRef rl, uint64_t id, bool one_shot, uint64_t milli_secs)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | (one_shot ? EV_ONESHOT : 0); 
    struct kevent change;
    struct kevent* change_ptr;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        change_ptr = &change;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int runloop_cancel_timer(RunloopRef rl, uint64_t id)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change;
    struct kevent* change_ptr;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        change_ptr = &change;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
/**
 * Register a RunloopWatcherBase (actuallyr one of its derivatives) and its associated file descriptor
 * with the epoll instance. Specify the types of events the watcher is interested in
 */
int runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_LOG_FMT("fd : %d  for events %d", fd, interest);
    // runloop_epoll_ctl(athis, EPOLL_CTL_ADD, fd, interest, wref);
    // FdTable_insert(athis->table, wref, fd);
    return 0;
}
int runloop_deregister(RunloopRef athis, int fd)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    // runloop_epoll_ctl(athis, EPOLL_CTL_DEL, fd, EPOLLEXCLUSIVE | EPOLLIN, NULL);
    // FdTable_remove(athis->table, fd);
    return 0;
}

int runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref) {
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    // runloop_epoll_ctl(athis, EPOLL_CTL_MOD, fd, interest, wref);
    RunloopWatcherBaseRef wref_tmp = FdTable_lookup(athis->table, fd);
    assert(wref == wref_tmp);
    return 0;
}
void runloop_delete(RunloopRef athis, int fd)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    FdTable_remove(athis->table, fd);
}
void print_events(struct kevent events[], int count)
{
    for(int i = 0; i < count; i++) {
        // struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int runloop_run(RunloopRef athis, time_t timeout) {
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

        if((FdTable_size(athis->table) == 0) 
            && ((functor_list_size(athis->ready_list) == 0))
            && (! event_allocator_has_outstanding_events(athis->event_allocator))
        ) {
            // no more work to do - clean exit
            result = 0;
            goto cleanup;
        }
        int max_events = runloop_MAX_EVENTS;
        if(functor_list_size(athis->ready_list) == 0) {
            struct timespec *timeout = NULL;
            struct  kevent* change = runloop_get_change_table(athis);
            int change_n = runloop_get_change_table_size(athis);
            struct kevent* events = runloop_get_fresh_event_table(athis);
            int max_events = runloop_get_max_events(athis);
            int nev = kevent(athis->kqueue_fd, change, change_n, events, max_events, timeout);
            RBL_LOG_FMT("runloop keventreturned nev: %d fd[0]: %d fds active: %ld  ready_list_size:%d",
                        nfds, events[0].data.fd,
                        FdTable_size(athis->table), functor_list_size(athis->ready_list));
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
                    goto cleanup;
                default: {
                    for (int i = 0; i < nev; i++) {
                        RUNLOOP_CHECK_TAG(athis)
                        RUNLOOP_CHECK_END_TAG(athis)
#if 1
                        struct kevent ee = athis->events[i];
                        void* pp = (void*)ee.ident;
                        RunloopWatcherBaseRef tr = pp;
                        RunloopWatcherBaseRef wr = athis->events[i].udata;
                        int fd = wr->fd;
#else
                        int fd = events[i].data.fd;
                        void *arg = events[i].data.ptr;
#endif
                        int mask = athis->events[i].filter;
                        RBL_LOG_FMT("runloop_run loop fd: %d events: %x", fd, mask);
                        // RunloopWatcherBaseRef wref = FdTable_lookup(athis->table, fd);
                        // assert(wr == wref);
                        wr->handler(wr, athis->events[i].filter);
                        RBL_LOG_FMT("fd: %d", fd);
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
