#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <c_http/list.h>
#include <c_http/xr/cbtable.h>
#include <c_http/xr/runloop.h>

#define MAX_EVENTS 4096

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

XrRunloopRef XrRunloop_new(void) {
    XrRunloopRef runloop;
    if ((runloop = malloc(sizeof(XrRunloop))) == NULL)
        abort();

    if ((runloop->epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1");
        free(runloop);
        return NULL;
    }
    runloop->table = CbTable_new();
    return runloop;
}

int XrRunloop_free(XrRunloopRef this) {
    if (close(this->epoll_fd) == -1) {
        perror("close");
        return -1;
    }
    int next_fd = CbTable_iterator(this->table);
    while (next_fd  != -1) {
        if (close(next_fd) == -1)
            return -1;
        next_fd = CbTable_next_iterator(this->table, next_fd);
    }
    CbTable_free(this->table);
    free(this);

    return 0;
}

//************************************************************

#define REACTOR_CTL(reactor, op, fd, interest)                                 \
    if (epoll_ctl(reactor->epoll_fd, op, fd,                                   \
                  &(struct epoll_event){.events = interest,                    \
                                        .data = {.fd = fd}}) == -1) {          \
        perror("epoll_ctl");                                                   \
        return -1;                                                             \
    }

int XrRunloop_register(XrRunloopRef this, int fd, uint32_t interest, XrWatcherRef wref)
{
    REACTOR_CTL(this, EPOLL_CTL_ADD, fd, interest)
    CbTable_insert(this->table, wref, fd);
    return 0;
}
int XrRunloop_deregister(XrRunloopRef reactor, int fd)
{
    REACTOR_CTL(reactor, EPOLL_CTL_DEL, fd, 0)
    CbTable_remove(reactor->table, fd);
    return 0;
}

int XrRunloop_reregister(XrRunloopRef this, int fd, uint32_t interest, XrWatcherRef wref) {
    REACTOR_CTL(this, EPOLL_CTL_MOD, fd, interest)
    CbTable_insert(this->table, wref, fd);
    return 0;
}

int static process_workq()
{
    return 0;
}
int static process_postq()
{
    return 0;
}
//************************************************************

int XrRunloop_run(XrRunloopRef this, time_t timeout) {
    int result;
    struct epoll_event *events;
    if ((events = calloc(MAX_EVENTS, sizeof(*events))) == NULL)
        abort();

    time_t start = time(NULL);

    while (true) {
        time_t passed = time(NULL) - start;
        int nfds = epoll_wait(this->epoll_fd, events, MAX_EVENTS, timeout - passed);
        time_t currtime = time(NULL);
        switch (nfds) {
            case -1:
                perror("epoll_wait");
                result = -1;
                goto cleanup;
            case 0:
                result = 0;
                goto cleanup;
            default: {
                for (int i = 0; i < nfds; i++) {
                    int fd = events[i].data.fd;
                    // call handler
                }
            }
        }
    }

cleanup:
    free(events);
    return result;
}
