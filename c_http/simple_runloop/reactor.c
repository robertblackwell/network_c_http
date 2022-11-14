#define XR_TRACE_ENABLEx
#define ENABLE_LOGx
#define _GNU_SOURCE
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <stdint.h>
#include <time.h>
#include <sys/epoll.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <c_http/logger.h>
#include <c_http/macros.h>
#include <c_http/common/list.h>
#include <c_http/async/types.h>

#define MAX_EVENTS 4096

static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}
/**
 * Performa a general epoll_ctl call with error checking.
 * In the event of an error abort 
 */
static void XrReactor_epoll_ctl(ReactorRef athis, int op, int fd, uint64_t interest)
{
    XR_REACTOR_CHECK_TAG(athis)
    struct epoll_event epev = {
        .events = interest,
        .data = {
            .fd = fd
        }
    };
    int status = epoll_ctl(athis->epoll_fd, op, fd, &(epev));
    if (status != 0) {
        LOG_FMT("XrReactor_epoll_ctl epoll_fd: %d status : %d errno : %d", this->epoll_fd, status, errno);
    }
    LOG_FMT("XrReactor_epoll_ctl epoll_fd: %d status : %d errno : %d", this->epoll_fd, status, errno);
    CHTTP_ASSERT((status == 0), "epoll ctl call failed");
}
/**
 * Create a new reactor runloop. Should only be one per thread
 * @TODO - store a runloop/reactor for each thread in thread local storage
 * @NOTE - this implementation only works for Linux and uses epoll
 */
ReactorRef XrReactor_new(void) {
    ReactorRef runloop = malloc(sizeof(Reactor));
    CHTTP_ASSERT((runloop != NULL), "malloc failed new simple_runloop");
    XR_REACTOR_SET_TAG(runloop)

    runloop->epoll_fd = epoll_create1(0);
    runloop->closed_flag = false;
    CHTTP_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    LOG_FMT("XrReactor_new epoll_fd %d", runloop->epoll_fd);
    runloop->table = FdTable_new();
    runloop->run_list = RunList_new();
    return (ReactorRef)runloop;
}

void XrReactor_close(ReactorRef athis)
{
    XR_REACTOR_CHECK_TAG(athis)
    athis->closed_flag = true;
    int status = close(athis->epoll_fd);
    LOG_FMT("XrReactor_close status: %d errno: %d", status, errno);
    CHTTP_ASSERT((status != -1), "close epoll_fd failed");
    int next_fd = FdTable_iterator(athis->table);
    while (next_fd  != -1) {
        close(next_fd);
        next_fd = FdTable_next_iterator(athis->table, next_fd);
    }
}

void XrReactor_free(ReactorRef athis)
{
    XR_REACTOR_CHECK_TAG(athis)
    if(! athis->closed_flag) {
        XrReactor_close(athis);
    }
    FdTable_free(athis->table);
    free(athis);
}

/**
 * Register a Watcher (actuallyr one of its derivatives) and its associated file descriptor
 * with the epoll instance. Specify the types of events the watcher is interested in
 */
int XrReactor_register(ReactorRef athis, int fd, uint32_t interest, WatcherRef wref)
{
    XR_REACTOR_CHECK_TAG(athis)
    LOG_FMT("fd : %d  for events %d", fd, interest);
    XrReactor_epoll_ctl (athis, EPOLL_CTL_ADD, fd, interest);
    FdTable_insert(athis->table, wref, fd);
    return 0;
}
int XrReactor_deregister(ReactorRef athis, int fd)
{
    XR_REACTOR_CHECK_TAG(athis)
    CHTTP_ASSERT((FdTable_lookup(athis->table, fd) != NULL),"fd not in FdTable");
    XrReactor_epoll_ctl(athis, EPOLL_CTL_DEL, fd, EPOLLEXCLUSIVE | EPOLLIN);
    FdTable_remove(athis->table, fd);
    return 0;
}

int XrReactor_reregister(ReactorRef athis, int fd, uint32_t interest, WatcherRef wref) {
    XR_REACTOR_CHECK_TAG(athis)
    CHTTP_ASSERT((FdTable_lookup(athis->table, fd) != NULL),"fd not in FdTable");
    XrReactor_epoll_ctl(athis, EPOLL_CTL_MOD, fd, interest);
    WatcherRef wref_tmp = FdTable_lookup(athis->table, fd);
    assert(wref == wref_tmp);
    return 0;
}
void XrReactor_delete(ReactorRef athis, int fd)
{
    XR_REACTOR_CHECK_TAG(athis)
    CHTTP_ASSERT((FdTable_lookup(athis->table, fd) != NULL),"fd not in FdTable");
    FdTable_remove(athis->table, fd);
}
void print_events(struct epoll_event events[], int count)
{
    for(int i = 0; i < count; i++) {
        struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int XrReactor_run(ReactorRef athis, time_t timeout) {
    XR_REACTOR_CHECK_TAG(athis)
    int result;
    struct epoll_event events[MAX_EVENTS];

    time_t start = time(NULL);

    while (true) {
        time_t passed = time(NULL) - start;
        if(FdTable_size(athis->table) == 0) {
            LOG_FMT("XrReactor_run() FdTable_size == 0");
            goto cleanup;
        }
        int max_events = MAX_EVENTS;
        /**
         * All entries on the runllist should be executed before we look for more fd events
         */
        assert(RunList_iterator(athis->run_list) == NULL);
        int nfds = epoll_wait(athis->epoll_fd, events, max_events, -1);
        time_t currtime = time(NULL);
        switch (nfds) {
            case -1:
                if(athis->closed_flag) {
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
                    int fd = events[i].data.fd;
                    int mask = events[i].events;
                    LOG_FMT("XrReactor_run loop fd: %d events: %x", fd, mask);
                    WatcherRef wref = FdTable_lookup(athis->table, fd);
                    wref->handler((void*)wref, fd, events[i].events);
                    LOG_FMT("fd: %d", fd);
                    // call handler
                }
            }
        }
        /**
         * @TODO - the loop should be changed to continually remove the front entry and process it until the list is empty.
         * Invariant - the runlist should never have more than 1 entry for any given file desacriptor.
         * This is currently not being checked. Need a paranoid option that checks it
         */
#if 1
//        fd_map_init();
        FunctorRef fnc;
        while(fnc = RunList_remove_first(athis->run_list)) {

            Functor_call(fnc);
        }
#else
        RunListIter iter = RunList_iterator(athis->run_list);
        while(iter != NULL) {
            FunctorRef fnc = RunList_itr_unpack(athis->run_list, iter);
            Functor_call(fnc);
            RunListIter next_iter = RunList_itr_next(athis->run_list, iter);
            RunList_itr_remove(athis->run_list, &iter);
            iter = next_iter;
        }
#endif
    }

cleanup:
    return result;
}

int XrReactor_post(ReactorRef athis, PostableFunction cb, void* arg)
{
    XR_REACTOR_CHECK_TAG(athis)
    FunctorRef fr = Functor_new(cb, arg);
    RunList_add_back(athis->run_list, fr);
}
