#include <c_http/runloop/w_iofd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

/**
 * Called whenever an fd associated with an WSocket receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(WatcherRef watcher, int fd, uint64_t event)
{
    WIoFdRef sw = (WIoFdRef)watcher;
    assert(fd == sw->fd);
    if((sw->event_mask & EPOLLIN) && (sw->read_evhandler)) {
        sw->read_evhandler(sw, sw->read_arg, event);
    }
    if((sw->event_mask & EPOLLOUT) && (sw->write_evhandler)) {
        sw->write_evhandler(sw, sw->write_arg, event);
    }
//    sw->cb((WatcherRef)sw, sw->cb_ctx, event);
}
static void anonymous_free(WatcherRef p)
{
    WIoFdRef twp = (WIoFdRef)p;
    WIoFd_free(twp);
}
void WIoFd_init(WIoFdRef this, ReactorRef runloop, int fd)
{
    this->type = XR_WATCHER_SOCKET;
    sprintf(this->tag, "XRSW");
    XR_SOCKW_SET_TAG(this);
    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->event_mask = 0;
    this->read_arg = NULL;
    this->read_evhandler = NULL;
    this->write_arg = NULL;
    this->write_evhandler = NULL;
}
WIoFdRef WIoFd_new(ReactorRef rtor_ref, int fd)
{
    WIoFdRef this = malloc(sizeof(WSocket));
    WIoFd_init(this, rtor_ref, fd);
    return this;
}
void WIoFd_free(WIoFdRef this)
{
    XR_SOCKW_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void WIoFd_register(WIoFdRef this)
{
    XR_SOCKW_CHECK_TAG(this)

    uint32_t interest = 0;
    int res = XrReactor_register(this->runloop, this->fd, 0L, (WatcherRef)(this));
    assert(res ==0);
}
//void WIoFd_change_watch(WIoFdRef this, SocketEventHandler cb, void* arg, uint64_t watch_what)
//{
//    XR_SOCKW_CHECK_TAG(this)
//    uint32_t interest = watch_what;
//    if( cb != NULL) {
//        this->cb = cb;
//    }
//    if (arg != NULL) {
//        this->cb_ctx = arg;
//    }
//    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
//    assert(res == 0);
//}
void WIoFd_deregister(WIoFdRef this)
{
    XR_SOCKW_CHECK_TAG(this)

    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
void WIoFd_arm_read(WIoFdRef this, SocketEventHandler fd_event_handler, void* arg)
{
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP | this->event_mask;
    this->event_mask = interest;
    XR_SOCKW_CHECK_TAG(this)
    if( fd_event_handler != NULL) {
        this->read_evhandler = fd_event_handler;
    }
    if (arg != NULL) {
        this->read_arg = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WIoFd_arm_write(WIoFdRef this, SocketEventHandler fd_event_handler, void* arg)
{
    uint64_t interest = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP | this->event_mask;
    this->event_mask = interest;
    XR_SOCKW_CHECK_TAG(this)
    if( fd_event_handler != NULL) {
        this->write_evhandler = fd_event_handler;
    }
    if (arg != NULL) {
        this->write_arg = arg;
    }
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    assert(res == 0);
}
void WIoFd_disarm_read(WIoFdRef this)
{
    this->event_mask &= ~EPOLLIN;
    XR_SOCKW_CHECK_TAG(this)
    this->read_evhandler = NULL;
    this->read_arg = NULL;
    int res = XrReactor_reregister(this->runloop, this->fd, this->event_mask, (WatcherRef)this);
    assert(res == 0);
}
void WIoFd_disarm_write(WIoFdRef this)
{
    this->event_mask = ~EPOLLOUT & this->event_mask;
    XR_SOCKW_CHECK_TAG(this)
    int res = XrReactor_reregister(this->runloop, this->fd, this->event_mask, (WatcherRef)this);
    assert(res == 0);
}


