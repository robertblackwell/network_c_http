#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <c_http/list.h>
#include <c_http/xr/types.h>
#include <c_http/xr/cbtable.h>
#include <c_http/xr/runloop.h>

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


struct XrRunloop_s {
    int               epoll_fd;
    CbTableRef        table; // (int, CallbackData)
};

static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}
static void XrRunloop_epoll_ctl(XrRunloopRef this, int op, int fd, uint64_t interest)
{
    struct epoll_event epev = {
        .events = interest,
        .data = {
            .fd = fd
        }
    };
    int status = epoll_ctl(this->epoll_fd, op, fd, &(epev));
    if (status != 0) {
        XR_PRINTF("XrRunloop_epoll_ctl epoll_fd: %d status : %d errno : %d\n", this->epoll_fd, status, errno);
    }
    XR_PRINTF("XrRunloop_epoll_ctl epoll_fd: %d status : %d errno : %d\n", this->epoll_fd, status, errno);
    XR_ASSERT((status == 0), "epoll ctl call failed");
}

XrRunloopRef XrRunloop_new(void) {
    XrRunloopRef runloop = malloc(sizeof(XrRunloop));
    XR_ASSERT((runloop != NULL), "malloc failed new runloop");

    runloop->epoll_fd = epoll_create1(0);
    XR_ASSERT((runloop->epoll_fd != -1), "epoll_create failed");
    XR_PRINTF("XrRunloop_new epoll_fd %d\n", runloop->epoll_fd);
    runloop->table = CbTable_new();
    return runloop;
}

void XrRunloop_free(XrRunloopRef this)
{
    int status = close(this->epoll_fd);
    XR_PRINTF("XrRunloop_free status: %d errno: %d \n", status, errno);
    XR_ASSERT((status != -1), "close epoll_fd failed");
    int next_fd = CbTable_iterator(this->table);
    while (next_fd  != -1) {
        close(next_fd);
        next_fd = CbTable_next_iterator(this->table, next_fd);
    }
    CbTable_free(this->table);
    free(this);
}


int XrRunloop_register(XrRunloopRef this, int fd, uint32_t interest, XrWatcherRef wref)
{
    XR_RL_CTL(this, EPOLL_CTL_ADD, fd, interest)
    CbTable_insert(this->table, wref, fd);
    return 0;
}
int XrRunloop_deregister(XrRunloopRef this, int fd)
{
    XR_ASSERT((CbTable_lookup(this->table, fd) != NULL),"fd not in CbTable");
//    XR_RL_CTL(this, EPOLL_CTL_DEL, fd, 0)
    XrRunloop_epoll_ctl(this, EPOLL_CTL_DEL, fd, 0);
    CbTable_remove(this->table, fd);
    return 0;
}

int XrRunloop_reregister(XrRunloopRef this, int fd, uint32_t interest, XrWatcherRef wref) {
    XR_ASSERT((CbTable_lookup(this->table, fd) != NULL),"fd not in CbTable");
    XR_RL_CTL(this, EPOLL_CTL_MOD, fd, interest)
    // entry must already be in table
    XrWatcherRef wref_tmp = CbTable_lookup(this->table, fd);
    assert(wref == wref_tmp);
    return 0;
}

int XrRunloop_run(XrRunloopRef this, time_t timeout) {
    int result;
    struct epoll_event *events;
    if ((events = calloc(MAX_EVENTS, sizeof(*events))) == NULL)
        abort();

    time_t start = time(NULL);

    while (true) {
        time_t passed = time(NULL) - start;
        // test to see if watcher list is empty - in which case exit loop
        if(CbTable_size(this->table) == 0) {
//            close(this->epoll_fd);
            XR_PRINTF("XrRunloop_run() CbTable_size == 0");
            goto cleanup;
        }
        int nfds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, -1);
        time_t currtime = time(NULL);
        switch (nfds) {
            case -1:
                perror("epoll_wait");
                result = -1;
                goto cleanup;
            case 0:
                result = 0;
                close(this->epoll_fd);
                goto cleanup;
            default: {
                for (int i = 0; i < nfds; i++) {
                    int fd = events[i].data.fd;
                    XR_PRINTF("XrRunloop_run loop fd: %d\n", fd);
                    XrWatcherRef wref = CbTable_lookup(this->table, fd);
                    wref->handler((void*)wref, fd, events[i].events);
                    XR_PRINTF("fd: %d\n", fd);
                    // call handler
                }
            }
        }
    }

cleanup:
    free(events);
    return result;
}
