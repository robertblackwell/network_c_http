#include <c_http/runloop/w_listener.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>


/**
 * Called whenever an fd associated with an WListener receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(WatcherRef watcher, int fd, uint64_t event)
{
    WListenerFdRef listener_ref = (WListenerFdRef)watcher;
    assert(fd ==  listener_ref->fd);
    if(listener_ref->listen_evhandler) {
        listener_ref->listen_evhandler(listener_ref,  listener_ref->listen_arg, event);
    }
}
static void anonymous_free(WatcherRef p)
{
    WListenerFdRef twp = (WListenerFdRef)p;
    WListenerFd_free(twp);
}
void WListenerFd_init(WListenerFdRef this, ReactorRef runloop, int fd)
{
    this->type = XR_WATCHER_LISTENER;
    XR_LISTNER_SET_TAG(this);
    this->fd = fd;
    this->runloop = runloop;
    this->free = &anonymous_free;
    this->handler = &handler;
    this->listen_arg = NULL;
    this->listen_evhandler = NULL;
}
WListenerFdRef WListenerFd_new(ReactorRef rtor_ref, int fd)
{
    WListenerFdRef this = malloc(sizeof(WListenerFd));
    WListenerFd_init(this, rtor_ref, fd);
    return this;
}
void WListenerFd_free(WListenerFdRef this)
{
    XR_LISTNER_CHECK_TAG(this)
    close(this->fd);
    free((void*)this);
}
void WListenerFd_register(WListenerFdRef this, ListenerEventHandler event_handler, void* arg)
{
    XR_LISTNER_CHECK_TAG(this)
    if( event_handler != NULL) {
        this->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        this->listen_arg = arg;
    }

    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    int res = XrReactor_register(this->runloop, this->fd, interest, (WatcherRef)(this));
    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res ==0);
}
void WListenerFd_deregister(WListenerFdRef this)
{
    XR_LISTNER_CHECK_TAG(this)
    XrReactor_delete(this->runloop, this->fd);
}
void WListenerFd_arm(WListenerFdRef this, ListenerEventHandler event_handler, void* arg)
{
    XR_LISTNER_CHECK_TAG(this)
    if( event_handler != NULL) {
        this->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        this->listen_arg = arg;
    }
    uint32_t interest = EPOLLIN | EPOLLEXCLUSIVE ;

    int res = XrReactor_reregister(this->runloop, this->fd, interest, (WatcherRef)this);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void WListenerFd_disarm(WListenerFdRef this)
{
    int res = XrReactor_reregister(this->runloop, this->fd, 0L, (WatcherRef)this);
    assert(res == 0);
}