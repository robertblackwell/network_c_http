#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <c_http/list.h>
#include <c_http/reactor/cbtable.h>
#include <c_http/reactor/timers.h>
#include <c_http/reactor/reactor.h>

#define MAX_EVENTS 4096

struct reactor {
    int               epoll_fd;
    CbTableRef        table; // (int, CallbackData)
    int               timers_fd;
    RctorTimerListRef timer_list;
    int               workq_fd;
    int               postq_fd;
};

static int *int_in_heap(int key) {
    int *result;
    if ((result = malloc(sizeof(*result))) == NULL)
        abort();
    *result = key;
    return result;
}

ReactorRef reactor_new(void) {
    Reactor *reactor;
    if ((reactor = malloc(sizeof(*reactor))) == NULL)
        abort();

    if ((reactor->epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1");
        free(reactor);
        return NULL;
    }

    reactor->table = CbTable_new();
    return reactor;
}

int reactor_destroy(Reactor *reactor) {
    if (close(reactor->epoll_fd) == -1) {
        perror("close");
        return -1;
    }

    int next_fd = CbTable_iterator(reactor->table);
    while (next_fd  != -1) {
        if (close(next_fd) == -1)
            return -1;
        next_fd = CbTable_next_iterator(reactor->table, next_fd);
    }

    CbTable_free(reactor->table);
    free(reactor);

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

int reactor_register(ReactorRef reactor,
                    int fd,
                    uint32_t interest,
                    Callback callback,
                    void *callback_arg) {
    REACTOR_CTL(reactor, EPOLL_CTL_ADD, fd, interest)
    CbTable_insert(reactor->table, callback, callback_arg, fd);
    return 0;
}
int reactor_deregister(ReactorRef reactor, int fd)
{
    REACTOR_CTL(reactor, EPOLL_CTL_DEL, fd, 0)
    CbTable_remove(reactor->table, fd);
    return 0;
}

int reactor_reregister(ReactorRef reactor, int fd, uint32_t interest, Callback callback, void *callback_arg) {
    REACTOR_CTL(reactor, EPOLL_CTL_MOD, fd, interest)
    CbTable_insert(reactor->table, callback, callback_arg, fd);
    return 0;
}


int reactor_timer_ctl(ReactorRef reactor, int op, uint32_t interest)
{
    int fd = reactor->timer_list->fd;
    struct epoll_event epevent = {
    .events = interest,
    .data = {.fd = fd}
    };
    if (epoll_ctl(reactor->epoll_fd, op, fd, &epevent ) == -1) {
        perror("epoll_ctl");
        return -1;
    }
}
RctorTimerRef reactor_register_timer(ReactorRef reactor, time_t when, Callback cb, void* arg)
{
    uint32_t  interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    int op = EPOLL_CTL_ADD;
    if (reactor_timer_ctl(reactor, op, interest) == -1) return -1;
    RctorTimerRef tr = RTimer_new();
    tr->expiry_time = when;
    tr->cb_data.arg = arg;
    tr->cb_data.callback = cb;
    ListRef list = reactor->timer_list->list;
    List_add_back(list, (void*)tr);
    return tr;
}
int reactor_deregister_timer(ReactorRef reactor)
{
    uint32_t  interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    int op = EPOLL_CTL_ADD;
    int fd = reactor->timer_list->fd;
    struct epoll_event epevent = {
    .events = interest,
    .data = {.fd = fd}
    };
    if (epoll_ctl(reactor->epoll_fd, op, fd, &epevent ) == -1) {
        perror("epoll_ctl");
        return -1;
    }
}
int reactor_reregister_timer(ReactorRef reactor, Callback cb, void* arg)
{
    uint32_t  interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    int op = EPOLL_CTL_ADD;
    int fd = reactor->timer_list->fd;
    struct epoll_event epevent = {
    .events = interest,
    .data = {.fd = fd}
    };
    if (epoll_ctl(reactor->epoll_fd, op, fd, &epevent ) == -1) {
        perror("epoll_ctl");
        return -1;
    }
}



int static process_timers(ReactorRef reactor)
{
    time_t curr_time = time(NULL);
    ListRef tl = reactor->timer_list->list;
    ListIterator iter = List_iterator(tl);
    while(iter != NULL) {
        RctorTimerRef tref = ((RctorTimerRef)List_itr_unpack(tl, iter));
        CallbackData cbd = tref->cb_data;
        time_t xt = tref->expiry_time;
        double dif = difftime(xt, curr_time);
        if(dif < 0.0) {
            tref->cb_data.callback(tref->cb_data.arg, reactor->timers_fd, 1);
            List_itr_remove(tl, iter);
        }
        iter = List_itr_next(tl, iter);
    }
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

int reactor_run(ReactorRef reactor, time_t timeout) {
    int result;
    struct epoll_event *events;
    if ((events = calloc(MAX_EVENTS, sizeof(*events))) == NULL)
        abort();

    time_t start = time(NULL);

    while (true) {
        time_t passed = time(NULL) - start;
        int nfds = epoll_wait(reactor->epoll_fd, events, MAX_EVENTS, timeout - passed);
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
                // call event handlers
                bool timers_flag = false;
                bool pollq_flag = false;
                bool workq_flag = false;
                for (int i = 0; i < nfds; i++) {
                    int fd = events[i].data.fd;
                    if (fd == reactor->timers_fd) {
                        timers_flag = true;
                    } else if (fd == reactor->postq_fd) {
                        process_postq();
                    } else if (fd == reactor->workq_fd) {
                        workq_flag = true;
                    } else {
                        CallbackData *callback = CbTable_lookup(reactor->table, fd);
                        callback->callback(callback->arg, fd, events[i].events);
                    }
                }
                if (timers_flag) {
                    process_timers(reactor);
                }
                if (workq_flag) {
                    process_workq();
                }
            }
        }
    }

cleanup:
    free(events);
    return result;
}
