#define XR_TRACE_ENABLEx
#define ENABLE_LOGx

#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
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
#include <http_in_c/common/list.h>

__thread ReactorRef my_reactor_ptr = NULL;

#define CHECK_THREAD(reactor_ref) //assert(reactor_ref == my_reactor_ptr);



static void drain_callback(void* arg)
{
    printf("drain callback\n");
}
static void interthread_queue_handler(RtorWQueueRef watcher, uint64_t event)
{
    printf("interthread_queue_handler\n");
    ReactorRef rx = rtor_wqueue_get_reactor(watcher);
    EvfdQueueRef evqref = watcher->queue;
    Functor func = Evfdq_remove(evqref);
    void* pf = func.f;
    watcher->queue_event_handler_arg = func.arg;
    void* arg = (void*) watcher;
    long d = (long) func.arg;
    printf("reactor::interthread_queue_handler f: %p d: %ld \n", pf, d);
    rtor_reactor_post(rx, func.f, arg);
}
static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}
/**
 * Perform a general epoll_ctl call with error checking.
 * In the event of an error abort 
 */
static void rtor_epoll_ctl(ReactorRef athis, int op, int fd, uint64_t interest)
{
    REACTOR_CHECK_TAG(athis)
    struct epoll_event epev = {
        .events = interest,
        .data = {
            .fd = fd
        }
    };
    int status = epoll_ctl(athis->epoll_fd, op, fd, &(epev));
    if (status != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("rtor_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", athis->epoll_fd, fd, status, errno_saved, strerror(errno_saved));
    }
    RBL_LOG_FMT("rtor_epoll_ctl epoll_fd: %d status : %d errno : %d", athis->epoll_fd, status, errno);
    RBL_ASSERT((status == 0), "epoll ctl call failed");
}

ReactorRef rtor_reactor_get_threads_reactor()
{
    return my_reactor_ptr;
}

/**
 * Create a new reactor runloop. Should only be one per thread
 * @NOTE - this implementation only works for Linux and uses epoll
 */
ReactorRef rtor_reactor_new(void) {
    ReactorRef runloop = malloc(sizeof(Reactor));
    RBL_ASSERT((runloop != NULL), "malloc failed new runloop");
    rtor_reactor_init(runloop);
    return (ReactorRef)runloop;
}
void rtor_reactor_init(ReactorRef athis) {
//    assert(my_reactor_ptr == NULL);
    my_reactor_ptr = athis;
    ReactorRef runloop = athis;

    REACTOR_SET_TAG(runloop)
    runloop->epoll_fd = epoll_create1(0);
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    RBL_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    RBL_LOG_FMT("rtor_reactor_new epoll_fd %d", runloop->epoll_fd);
    runloop->table = FdTable_new();
    runloop->ready_list = functor_list_new(RTOR_READY_LIST_MAX);
    runloop->interthread_queue_ref = NULL;
    runloop->interthread_queue_watcher_ref = NULL;
}
void rtor_reactor_enable_interthread_queue(ReactorRef rtor_ref)
{
    rtor_ref->interthread_queue_ref = Evfdq_new();
    rtor_ref->interthread_queue_watcher_ref = rtor_wqueue_new(rtor_ref, rtor_ref->interthread_queue_ref);
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    rtor_wqueue_register(rtor_ref->interthread_queue_watcher_ref, &interthread_queue_handler,
                         (void *) rtor_ref->interthread_queue_ref);
}
void rtor_reactor_close(ReactorRef athis)
{
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    athis->closed_flag = true;
    int status = close(athis->epoll_fd);
    RBL_LOG_FMT("rtor_reactor_close status: %d errno: %d", status, errno);
    RBL_ASSERT((status != -1), "close epoll_fd failed");
    int next_fd = FdTable_iterator(athis->table);
    while (next_fd  != -1) {
        close(next_fd);
        next_fd = FdTable_next_iterator(athis->table, next_fd);
    }
}

void rtor_reactor_free(ReactorRef athis)
{
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    if(! athis->closed_flag) {
        rtor_reactor_close(athis);
    }
    if(athis->interthread_queue_ref) {
        Evfdq_free(athis->interthread_queue_ref);
    }
    if(athis->interthread_queue_watcher_ref) {
        rtor_wqueue_dispose(&(athis->interthread_queue_watcher_ref));
    }
    FdTable_free(athis->table);
    functor_list_free(athis->ready_list);
    free(athis);
}

/**
 * Register a RtorWatcher (actuallyr one of its derivatives) and its associated file descriptor
 * with the epoll instance. Specify the types of events the watcher is interested in
 */
int rtor_reactor_register(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref)
{
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    RBL_LOG_FMT("fd : %d  for events %d", fd, interest);
    rtor_epoll_ctl(athis, EPOLL_CTL_ADD, fd, interest);
    FdTable_insert(athis->table, wref, fd);
    return 0;
}
int rtor_reactor_deregister(ReactorRef athis, int fd)
{
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    rtor_epoll_ctl(athis, EPOLL_CTL_DEL, fd, EPOLLEXCLUSIVE | EPOLLIN);
    FdTable_remove(athis->table, fd);
    return 0;
}

int rtor_reactor_reregister(ReactorRef athis, int fd, uint32_t interest, RtorWatcherRef wref) {
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    rtor_epoll_ctl(athis, EPOLL_CTL_MOD, fd, interest);
    RtorWatcherRef wref_tmp = FdTable_lookup(athis->table, fd);
    assert(wref == wref_tmp);
    return 0;
}
void rtor_reactor_delete(ReactorRef athis, int fd)
{
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    FdTable_remove(athis->table, fd);
}
void print_events(struct epoll_event events[], int count)
{
    for(int i = 0; i < count; i++) {
        struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int rtor_reactor_run(ReactorRef athis, time_t timeout) {
    REACTOR_CHECK_TAG(athis)
    CHECK_THREAD(athis)
    int result;
    struct epoll_event events[RTOR_MAX_EPOLL_FDS];

    time_t start = time(NULL);

    while (true) {
        REACTOR_CHECK_TAG(athis)
        time_t passed = time(NULL) - start;
        if(FdTable_size(athis->table) == 0) {
            goto cleanup;
        }
        if((FdTable_size(athis->table) == 1) && (athis->interthread_queue_ref != NULL)) {
            rtor_wqueue_deregister(athis->interthread_queue_watcher_ref);
            goto cleanup;
        }
        int max_events = RTOR_MAX_EPOLL_FDS;
        /**
         * All entries on the ready should be executed before we look for more fd events
         */
        assert(functor_list_size(athis->ready_list) == 0);
        RBL_LOG_FMT("reactor epoll_wait before active fd : %ld ready_list_size: %d",
                    FdTable_size(athis->table), functor_list_size(athis->ready_list));
        int nfds = epoll_wait(athis->epoll_fd, events, max_events, -1);
        RBL_LOG_FMT("reactor epoll_wait returned nfds: %d fd[0]: %d fds active: %ld  ready_list_size:%d",
                    nfds, events[0].data.fd,
                    FdTable_size(athis->table), functor_list_size(athis->ready_list));
        time_t currtime = time(NULL);
        switch (nfds) {
            case -1:
                if (errno == EINTR) {
                    printf("reactor interripted\n");
                    goto cleanup;
                    continue;
                } else if(athis->closed_flag) {
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
                goto cleanup;
            default: {
                for (int i = 0; i < nfds; i++) {
                    REACTOR_CHECK_TAG(athis)
                    int fd = events[i].data.fd;
                    int mask = events[i].events;
                    RBL_LOG_FMT("rtor_reactor_run loop fd: %d events: %x", fd, mask);
                    RtorWatcherRef wref = FdTable_lookup(athis->table, fd);
                    wref->handler(wref, events[i].events);
                    RBL_LOG_FMT("fd: %d", fd);
                    // call handler
                    REACTOR_CHECK_TAG(athis)
                }
            }
        }
        FunctorRef fnc;
        while(1) {
            REACTOR_CHECK_TAG(athis)
            if(functor_list_size(athis->ready_list) == 0) {
                break;
            }
            Functor func = functor_list_remove(athis->ready_list);
            athis->runloop_executing = true;
            func.f(athis, func.arg);
            athis->runloop_executing = false;
            REACTOR_CHECK_TAG(athis)
            if(functor_list_size(athis->ready_list) == 0) {
                RBL_LOG_FMT("reactor runlist loop  break functor_list_size: %d func: %p arg: %p", functor_list_size(athis->ready_list), func.f, func.arg);
                break;
            }
        }
//        printf("reactor after runlist loop functor_list_size: %d\n", functor_list_size(athis->ready_list));
//        while(fnc = RunList_remove_first(athis->ready_list)) {
//
//            Functor_call(fnc, athis);
//            Functor_free(fnc);
//        }
    }

cleanup:
    return result;
}

int rtor_reactor_post(ReactorRef athis, PostableFunction cb, void* arg)
{
    REACTOR_CHECK_TAG(athis)
    RBL_LOG_FMT("rtor_reactor_post entered functor_list_size: %d funct: %p arg: %p runloop_executing: %d\n", functor_list_size(athis->ready_list), cb, arg, (int)athis->runloop_executing);
    Functor func = {.f = cb, .arg = arg};
    functor_list_add(athis->ready_list, func);
    RBL_LOG_FMT("rtor_reactor_post exited functor_list_size: %d func: %p arg: %p runloop_executing: %d\n", functor_list_size(athis->ready_list), cb, arg, (int)athis->runloop_executing);
}

void rtor_reactor_interthread_post(ReactorRef athis, PostableFunction cb, void* arg)
{
    REACTOR_CHECK_TAG(athis)
    Functor func = {.f = cb, .arg = arg};
    Evfdq_add(athis->interthread_queue_ref, func);
#if 1
#else
    rtor_interthread_queue_add(athis->interthread_queue, fr);
#endif
}
