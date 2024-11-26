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
/**
 * Perform a general epoll_ctl call with error checking.
 * In the event of an error abort 
 */
static void runloop_epoll_ctl(RunloopRef athis, int op, int fd, uint64_t interest, void* watcher)
{
    RUNLOOP_CHECK_TAG(athis)
    struct epoll_event epev = {
        .events = interest,
        .data = {
            .fd = fd,
        }
    };
    // note epev.data is a union so the next line overites .fd = fd
    epev.data.ptr = watcher;
    int status = epoll_ctl(athis->epoll_fd, op, fd, &(epev));
    if (status != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("runloop_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", athis->epoll_fd, fd, status, errno_saved, strerror(errno_saved));
    }
    RBL_LOG_FMT("runloop_epoll_ctl epoll_fd: %d status : %d errno : %d", athis->epoll_fd, status, errno);
    RBL_ASSERT((status == 0), "epoll ctl call failed");
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
    runloop->epoll_fd = epoll_create1(0);
    runloop->closed_flag = false;
    runloop->runloop_executing = false;
    RBL_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    RBL_LOG_FMT("runloop_new epoll_fd %d", runloop->epoll_fd);
    runloop->table = FdTable_new();
    runloop->ready_list = functor_list_new(runloop_READY_LIST_MAX);
}
void runloop_close(RunloopRef athis)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    athis->closed_flag = true;
    int status = close(athis->epoll_fd);
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

/**
 * Register a RunloopWatcherBase (actuallyr one of its derivatives) and its associated file descriptor
 * with the epoll instance. Specify the types of events the watcher is interested in
 */
int runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_LOG_FMT("fd : %d  for events %d", fd, interest);
    runloop_epoll_ctl(athis, EPOLL_CTL_ADD, fd, interest, wref);
    FdTable_insert(athis->table, wref, fd);
    return 0;
}
int runloop_deregister(RunloopRef athis, int fd)
{
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    runloop_epoll_ctl(athis, EPOLL_CTL_DEL, fd, EPOLLEXCLUSIVE | EPOLLIN, NULL);
    FdTable_remove(athis->table, fd);
    return 0;
}

int runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref) {
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
    RBL_ASSERT((FdTable_lookup(athis->table, fd) != NULL), "fd not in FdTable");
    runloop_epoll_ctl(athis, EPOLL_CTL_MOD, fd, interest, wref);
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
void print_events(struct epoll_event events[], int count)
{
    for(int i = 0; i < count; i++) {
        struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int runloop_run(RunloopRef athis, time_t timeout) {
    RUNLOOP_CHECK_TAG(athis)
    RUNLOOP_CHECK_END_TAG(athis)
//    athis->tid = gettid();
    int result;
    struct epoll_event events[runloop_MAX_EPOLL_FDS];

    time_t start = time(NULL);

    while (true) {
        RUNLOOP_CHECK_TAG(athis)
        RUNLOOP_CHECK_END_TAG(athis)
        time_t passed = time(NULL) - start;

        if((FdTable_size(athis->table) == 0) &&((functor_list_size(athis->ready_list) == 0))) {
            // no more work to do - clean exit
            result = 0;
            goto cleanup;
        }
        int max_events = runloop_MAX_EPOLL_FDS;
        if(functor_list_size(athis->ready_list) == 0) {
            int nfds = epoll_wait(athis->epoll_fd, events, max_events, -1);
            RBL_LOG_FMT("reactor epoll_wait returned nfds: %d fd[0]: %d fds active: %ld  ready_list_size:%d",
                        nfds, events[0].data.fd,
                        FdTable_size(athis->table), functor_list_size(athis->ready_list));
            time_t currtime = time(NULL);
            switch (nfds) {
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
                        RUNLOOP_CHECK_TAG(athis)
                        RUNLOOP_CHECK_END_TAG(athis)
#if 1
                        RunloopWatcherBaseRef wr = events[i].data.ptr;
                        int fd = wr->fd;
#else
                        int fd = events[i].data.fd;
                        void *arg = events[i].data.ptr;
#endif
                        int mask = events[i].events;
                        RBL_LOG_FMT("runloop_run loop fd: %d events: %x", fd, mask);
                        RunloopWatcherBaseRef wref = FdTable_lookup(athis->table, fd);
                        assert(wr == wref);
                        wref->handler(wref, events[i].events);
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
