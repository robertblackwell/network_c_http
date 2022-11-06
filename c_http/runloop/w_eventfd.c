#include <c_http/runloop/w_eventfd.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <c_http/async/types.h>

#define TWO_PIPE_TRICKx
#define SEMAPHORE

/**
 *
 * @param ctx
 * @param fd
 * @param event
 */
static void handler(WatcherRef fdevent_ref, int fd, uint64_t event)
{
    WFdEventRef fdev = (WFdEventRef)fdevent_ref;
    XR_FDEV_CHECK_TAG(fdev)
    uint64_t buf;
    int nread = read(fdev->fd, &buf, sizeof(buf));
    printf("XXXX value read from fdevent fd: nread %d value %ld\n", nread, buf);
    assert(fd == fdev->fd);
    if(nread == sizeof(buf)) {
        fdev->fd_event_handler(fdev, fdev->fd_event_handler_arg, event);
    } else {

    }
}
static void anonymous_free(WatcherRef p)
{
    WFdEventRef fdevp = (WFdEventRef)p;
    XR_FDEV_CHECK_TAG(fdevp)
    WEventFd_free(fdevp);
}
void WEventFd_init(WFdEventRef this, ReactorRef runloop)
{
    this->type = XR_WATCHER_FDEVENT;
    XR_FDEV_SET_TAG(this);
    XR_FDEV_CHECK_TAG(this)
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
WFdEventRef WEventFd_new(ReactorRef rtor_ref)
{
    WFdEventRef this = malloc(sizeof(WFdEvent));
    WEventFd_init(this, rtor_ref);
    return this;
}
void WEventFd_free(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void WEventFd_register(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)

    uint32_t interest = 0L;
    this->fd_event_handler = NULL;
    this->fd_event_handler_arg = NULL;
    int res = XrReactor_register(this->runloop, this->fd, interest, (WatcherRef)(this));
    assert(res ==0);
}
void WEventFd_change_watch(WFdEventRef this, FdEventHandler evhandler, void* arg, uint64_t watch_what)
{
    XR_FDEV_CHECK_TAG(this)
    uint32_t interest = watch_what;
    if( evhandler != NULL) {
        this->fd_event_handler = evhandler;
    }
    if (arg != NULL) {
        this->fd_event_handler_arg = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WEventFd_deregister(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
void WEventFd_arm(WFdEventRef this, FdEventHandler evhandler, void* arg)
{
    XR_FDEV_CHECK_TAG(this)
    uint32_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP;
    if( evhandler != NULL) {
        this->fd_event_handler = evhandler;
    }
    if (arg != NULL) {
        this->fd_event_handler_arg = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WEventFd_disarm(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    int res = XrReactor_reregister(this->runloop, this->fd, 0, (WatcherRef)this);
}
void WEventFd_fire(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
#ifdef TWO_PIPE_TRICK
    uint64_t buf = 1;
    write(this->write_fd, &buf, sizeof(buf));
#else
    uint64_t buf = 1;
    int x = write(this->fd, &buf, sizeof(buf));
    assert(x == sizeof(buf));
#endif
}
