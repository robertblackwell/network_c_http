#include <c_http/xr/w_fdevent.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <c_http/xr/types.h>

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
    XRFD_TYPE_CHECK(fdev)
    uint64_t buf;
    int nread = read(fdev->fd, &buf, sizeof(buf));
    assert(fd == fdev->fd);
    fdev->fd_event_handler(fdev, fdev->fd_event_handler_arg, event);
}
static void anonymous_free(WatcherRef p)
{
    WFdEventRef fdevp = (WFdEventRef)p;
    XRFD_TYPE_CHECK(fdevp)
    XR_FDEV_CHECK_TAG(fdevp)

    WFdEvent_free(fdevp);
}
void WFdEvent_init(WFdEventRef this, XrReactorRef runloop)
{
    this->type = XR_WATCHER_FDEVENT;
    XR_FDEV_SET_TAG(this);
    XR_FDEV_CHECK_TAG(this)
//    this->fd = eventfd(0, O_NONBLOCK | O_CLOEXEC);
    int pipefds[2];
    pipe(pipefds);
    this->fd = pipefds[0];
    this->write_fd = pipefds[1];
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
}
WFdEventRef WFdEvent_new(XrReactorRef rtor_ref)
{
    WFdEventRef this = malloc(sizeof(WFdEvent));
    WFdEvent_init(this, rtor_ref);
    return this;
}
void WFdEvent_free(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
    close(this->fd);
    free((void*)this);
}
void WFdEvent_register(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)

    uint32_t interest = 0L;
    this->fd_event_handler = NULL;
    this->fd_event_handler_arg = NULL;
    int res = XrReactor_register(this->runloop, this->fd, interest, (WatcherRef)(this));
    assert(res ==0);
}
void WFdEvent_change_watch(WFdEventRef this, FdEventHandler evhandler, void* arg, uint64_t watch_what)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
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
void WFdEvent_deregister(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
void WFdEvent_arm(WFdEventRef this, FdEventHandler evhandler, void* arg)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
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
void WFdEvent_disarm(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
    int res = XrReactor_reregister(this->runloop, this->fd, 0, (WatcherRef)this);
}
void WFdEvent_fire(WFdEventRef this)
{
    XR_FDEV_CHECK_TAG(this)
    XRFD_TYPE_CHECK(this)
    uint64_t buf = 1;
    write(this->write_fd, &buf, sizeof(buf));
}
