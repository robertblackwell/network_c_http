#include <c_http/xr/socket_watcher.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

/**
 * Called whenever an fd associated with an XrSocketWatcher receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(XrWatcherRef watcher, int fd, uint64_t event)
{
    XrSocketWatcherRef sw = (XrSocketWatcherRef)watcher;
    assert(fd == sw->fd);
    if((sw->event_mask & EPOLLIN) && (sw->read_evhandler)) {
        sw->read_evhandler(sw, sw->read_arg, event);
    }
    if((sw->event_mask & EPOLLOUT) && (sw->write_evhandler)) {
        sw->write_evhandler(sw, sw->write_arg, event);
    }
//    sw->cb((XrWatcherRef)sw, sw->cb_ctx, event);
}
static void anonymous_free(XrWatcherRef p)
{
    XrSocketWatcherRef twp = (XrSocketWatcherRef)p;
    Xrsw_free(twp);
}
void Xrsw_init(XrSocketWatcherRef this, XrReactorRef runloop, int fd)
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
XrSocketWatcherRef Xrsw_new(XrReactorRef rtor_ref, int fd)
{
    XrSocketWatcherRef this = malloc(sizeof(XrSocketWatcher));
    Xrsw_init(this, rtor_ref, fd);
    return this;
}
void Xrsw_free(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void Xrsw_register(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)

    uint32_t interest = 0;
    int res = XrReactor_register(this->runloop, this->fd, 0L, (XrWatcherRef)(this));
    assert(res ==0);
}
//void Xrsw_change_watch(XrSocketWatcherRef this, SocketEventHandler cb, void* arg, uint64_t watch_what)
//{
//    XR_SOCKW_CHECK_TAG(this)
//    uint32_t interest = watch_what;
//    if( cb != NULL) {
//        this->cb = cb;
//    }
//    if (arg != NULL) {
//        this->cb_ctx = arg;
//    }
//    int res = XrReactor_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
//    assert(res == 0);
//}
void Xrsw_deregister(XrSocketWatcherRef this)
{
    XRSW_TYPE_CHECK(this)
    XR_SOCKW_CHECK_TAG(this)

    int res =  XrReactor_deregister(this->runloop, this->fd);
    assert(res == 0);
}
void Xrsw_arm_read(XrSocketWatcherRef this, SocketEventHandler fd_event_handler, void* arg)
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
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrsw_arm_write(XrSocketWatcherRef this, SocketEventHandler fd_event_handler, void* arg)
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
    int res = XrReactor_reregister(this->runloop, this->fd, interest, (XrWatcherRef)this);
    assert(res == 0);
}
void Xrsw_disarm_read(XrSocketWatcherRef this)
{

}
void Xrsw_disarm_write(XrSocketWatcherRef this)
{

}


