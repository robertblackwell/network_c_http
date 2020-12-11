#define XR_TRACE_ENABLE
#define XR_PRINTF_ENABLE
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <c_http/list.h>
#include <c_http/xr/types.h>
#include <c_http/xr/fdtable.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/run_list.h>

#define MAX_EVENTS 4096
/**
 * epoll_ctl call
 */
#define XR_RL_CTL(reactor, op, fd, interest)                                 \
    if (epoll_ctl(reactor->epoll_fd, op, fd,                                   \
                  &(struct epoll_event){.events = interest,                    \
                                        .data = {.fd = fd}}) == -1) {          \
        XR_PRINTF("XR_RL_CTL epoll_fd: %d \n", reactor->epoll_fd);                                                   \
        return -1;                                                             \
    }


struct XrReactor_s {
    XR_REACTOR_DECLARE_TAG;
    int               epoll_fd;
    FdTableRef        table; // (int, CallbackData)
    RunListRef        run_list;
};

static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}
static void XrReactor_epoll_ctl(XrReactorRef this, int op, int fd, uint64_t interest)
{
    XR_REACTOR_CHECK_TAG(this)
    struct epoll_event epev = {
        .events = interest,
        .data = {
            .fd = fd
        }
    };
    int status = epoll_ctl(this->epoll_fd, op, fd, &(epev));
    if (status != 0) {
        XR_PRINTF("XrReactor_epoll_ctl epoll_fd: %d status : %d errno : %d\n", this->epoll_fd, status, errno);
    }
    XR_PRINTF("XrReactor_epoll_ctl epoll_fd: %d status : %d errno : %d\n", this->epoll_fd, status, errno);
    XR_ASSERT((status == 0), "epoll ctl call failed");
}

XrReactorRef XrReactor_new(void) {
    XrReactorRef runloop = malloc(sizeof(XrReactor));
    XR_ASSERT((runloop != NULL), "malloc failed new runloop");
    XR_REACTOR_SET_TAG(runloop)

    runloop->epoll_fd = epoll_create1(0);
    XR_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    XR_PRINTF("XrReactor_new epoll_fd %d\n", runloop->epoll_fd);
    runloop->table = FdTable_new();
    runloop->run_list = RunList_new();
    return runloop;
}

void XrReactor_free(XrReactorRef this)
{
    XR_REACTOR_CHECK_TAG(this)
    int status = close(this->epoll_fd);
    XR_PRINTF("XrReactor_free status: %d errno: %d \n", status, errno);
    XR_ASSERT((status != -1), "close epoll_fd failed");
    int next_fd = FdTable_iterator(this->table);
    while (next_fd  != -1) {
        close(next_fd);
        next_fd = FdTable_next_iterator(this->table, next_fd);
    }
    FdTable_free(this->table);
    free(this);
}


int XrReactor_register(XrReactorRef this, int fd, uint32_t interest, XrWatcherRef wref)
{
    XR_REACTOR_CHECK_TAG(this)

    XR_PRINTF("fd : %d  for events %d\n", fd, interest);
    XrReactor_epoll_ctl (this, EPOLL_CTL_ADD, fd, interest);
    FdTable_insert(this->table, wref, fd);
    return 0;
}
int XrReactor_deregister(XrReactorRef this, int fd)
{
    XR_REACTOR_CHECK_TAG(this)
    XR_ASSERT((FdTable_lookup(this->table, fd) != NULL),"fd not in FdTable");
    XrReactor_epoll_ctl(this, EPOLL_CTL_DEL, fd, 0);
    FdTable_remove(this->table, fd);
    return 0;
}

int XrReactor_reregister(XrReactorRef this, int fd, uint32_t interest, XrWatcherRef wref) {
    XR_REACTOR_CHECK_TAG(this)
    XR_ASSERT((FdTable_lookup(this->table, fd) != NULL),"fd not in FdTable");
    XrReactor_epoll_ctl(this, EPOLL_CTL_MOD, fd, interest);
    XrWatcherRef wref_tmp = FdTable_lookup(this->table, fd);
    assert(wref == wref_tmp);
    return 0;
}
void print_events(struct epoll_event events[], int count)
{
    for(int i = 0; i < count; i++) {
        struct epoll_event *ev = &(events[i]);
        printf("\n");
    }
}
int XrReactor_run(XrReactorRef this, time_t timeout) {
    XR_REACTOR_CHECK_TAG(this)
    int result;
    struct epoll_event events[MAX_EVENTS];

    time_t start = time(NULL);

    while (true) {
        time_t passed = time(NULL) - start;
        // test to see if watcher list is empty - in which case exit loop
        if(FdTable_size(this->table) == 0) {
//            close(this->epoll_fd);
            XR_PRINTF("XrReactor_run() FdTable_size == 0");
            goto cleanup;
        }
        int max_events = MAX_EVENTS;
        int nfds = epoll_wait(this->epoll_fd, events, max_events, -1);
        time_t currtime = time(NULL);
        switch (nfds) {
            case -1:
                perror("epoll_wait");
                result = -1;
                int ern = errno;
                goto cleanup;
            case 0:
                result = 0;
                close(this->epoll_fd);
                goto cleanup;
            default: {
                for (int i = 0; i < nfds; i++) {
                    int fd = events[i].data.fd;
                    int mask = events[i].events;
                    XR_PRINTF("XrReactor_run loop fd: %d events: %x \n", fd, mask);
                    XrWatcherRef wref = FdTable_lookup(this->table, fd);
                    wref->handler((void*)wref, fd, events[i].events);
                    XR_PRINTF("fd: %d\n", fd);
                    // call handler
                }
            }
        }
        RunListIter iter = RunList_iterator(this->run_list);
        while(iter != NULL) {
            FunctorRef fnc = RunList_itr_unpack(this->run_list, iter);
            Functor_call(fnc);
            RunListIter next_iter = RunList_itr_next(this->run_list, iter);
            RunList_itr_remove(this->run_list, &iter);
            iter = next_iter;
        }
    }

cleanup:
    return result;
}

int XrReactor_post(XrReactorRef this, PostableFunction cb, void* arg)
{
    XR_REACTOR_CHECK_TAG(this)
    FunctorRef fr = Functor_new(cb, arg);
    RunList_add_back(this->run_list, fr);
}
