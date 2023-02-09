#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
//#include <http_in_c/async/types.h>

#define TWO_PIPE_TRICK
#define SEMAPHORE

/**
 *
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(RtorWatcherRef fdevent_ref, uint64_t event)
{
    RtorEventfdRef fdev = (RtorEventfdRef)fdevent_ref;
    FDEV_CHECK_TAG(fdev)
    uint64_t buf;
    int nread = read(fdev->fd, &buf, sizeof(buf));
    if(nread == sizeof(buf)) {
        fdev->fd_event_handler(fdev, event);
    } else {

    }
}
static void anonymous_free(RtorWatcherRef p)
{
    RtorEventfdRef fdevp = (RtorEventfdRef)p;
    FDEV_CHECK_TAG(fdevp)
    rtor_eventfd_free(fdevp);
}
void rtor_eventfd_init(RtorEventfdRef this, ReactorRef runloop)
{
    this->type = XR_WATCHER_FDEVENT;
    FDEV_SET_TAG(this);
    FDEV_CHECK_TAG(this)
#ifdef TWO_PIPE_TRICK
    int pipefds[2];
    pipe(pipefds);
    this->fd = pipefds[0];
    this->write_fd = pipefds[1];
#else
    #ifdef SEMAPHORE
        this->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    #else
        this->fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    #endif
#endif
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
RtorEventfdRef rtor_eventfd_new(ReactorRef runloop)
{
    RtorEventfdRef this = malloc(sizeof(RtorEventfd));
    rtor_eventfd_init(this, runloop);
    return this;
}
void rtor_eventfd_free(RtorEventfdRef athis)
{
    FDEV_CHECK_TAG(athis)
    close(athis->fd);
    free((void*)athis);
}
void rtor_eventfd_register(RtorEventfdRef athis)
{
    FDEV_CHECK_TAG(athis)

    uint32_t interest = 0L;
    athis->fd_event_handler = NULL;
    athis->fd_event_handler_arg = NULL;
    int res = rtor_reactor_register(athis->runloop, athis->fd, interest, (RtorWatcherRef) (athis));
    assert(res ==0);
}
void rtor_eventfd_change_watch(RtorEventfdRef athis, FdEventHandler evhandler, void* arg, uint64_t watch_what)
{
    FDEV_CHECK_TAG(athis)
    uint32_t interest = watch_what;
    if( evhandler != NULL) {
        athis->fd_event_handler = evhandler;
    }
    if (arg != NULL) {
        athis->fd_event_handler_arg = arg;
    }
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_eventfd_deregister(RtorEventfdRef athis)
{
    FDEV_CHECK_TAG(athis)
    int res = rtor_reactor_deregister(athis->runloop, athis->fd);
    assert(res == 0);
}
void rtor_eventfd_arm(RtorEventfdRef athis, FdEventHandler evhandler, void* arg)
{
    FDEV_CHECK_TAG(athis)
    uint32_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP;
    if( evhandler != NULL) {
        athis->fd_event_handler = evhandler;
    }
    if (arg != NULL) {
        athis->fd_event_handler_arg = arg;
    }
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    assert(res == 0);
}
void rtor_eventfd_disarm(RtorEventfdRef athis)
{
    FDEV_CHECK_TAG(athis)
    int res = rtor_reactor_reregister(athis->runloop, athis->fd, 0, (RtorWatcherRef) athis);
}
void rtor_eventfd_fire(RtorEventfdRef athis)
{
    FDEV_CHECK_TAG(athis)
#ifdef TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(athis->write_fd, &buf, sizeof(buf));
#else
    uint64_t buf = 1;
    int x = write(athis->fd, &buf, sizeof(buf));
    assert(x == sizeof(buf));
#endif
}
void rtor_eventfd_clear_one_event(RtorEventfdRef athis)
{
    uint64_t buf;
    int nread = read(athis->fd, &buf, sizeof(buf));
}
void rtor_eventfd_clear_all_events(RtorEventfdRef athis)
{
    uint64_t buf;
    while(1) {
        int nread = read(athis->fd,  &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);
}

ReactorRef rtor_eventfd_get_reactor(RtorEventfdRef athis)
{
    return athis->runloop;
}
int rtor_eventfd_get_fd(RtorEventfdRef this)
{
    return this->fd;
}

void rtor_eventfd_verify(RtorEventfdRef r)
{
    FDEV_CHECK_TAG(r)

}
