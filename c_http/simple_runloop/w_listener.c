#include <c_http/simple_runloop/runloop.h>
#include <c_http//simple_runloop/rl_internal.h>
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
    WListenerFd_free((WListenerFdRef)p);
}
void WListenerFd_init(WListenerFdRef athis, ReactorRef runloop, int fd)
{
    athis->type = XR_WATCHER_LISTENER;
    athis->fd = fd;
    athis->runloop = runloop;
    athis->free = &anonymous_free;
    athis->handler = &handler;
    athis->listen_arg = NULL;
    athis->listen_evhandler = NULL;
}
WListenerFdRef WListenerFd_new(ReactorRef rtor_ref, int fd)
{
    WListenerFdRef this = malloc(sizeof(WListenerFd));
    WListenerFd_init(this, rtor_ref, fd);
    return this;
}
void WListenerFd_free(WListenerFdRef athis)
{
    XR_LISTNER_CHECK_TAG(athis)
    close(athis->fd);
    free((void*)athis);
}
void WListenerFd_register(WListenerFdRef athis, ListenerEventHandler event_handler, void* arg)
{
    XR_LISTNER_CHECK_TAG(athis)
    if( event_handler != NULL) {
        athis->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }

    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    int res = XrReactor_register(athis->runloop, athis->fd, interest, (WatcherRef)(athis));
    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res ==0);
}
void WListenerFd_deregister(WListenerFdRef athis)
{
    XR_LISTNER_CHECK_TAG(athis)
    XrReactor_delete(athis->runloop, athis->fd);
}
void WListenerFd_arm(WListenerFdRef athis, ListenerEventHandler event_handler, void* arg)
{
    XR_LISTNER_CHECK_TAG(athis)
    if( event_handler != NULL) {
        athis->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }
    uint32_t interest = EPOLLIN | EPOLLEXCLUSIVE ;

    int res = XrReactor_reregister(athis->runloop, athis->fd, interest, (WatcherRef)athis);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void WListenerFd_disarm(WListenerFdRef athis)
{
    XR_LISTNER_CHECK_TAG(athis)
    int res = XrReactor_reregister(athis->runloop, athis->fd, 0L, (WatcherRef)athis);
    assert(res == 0);
}
ReactorRef WListenerFd_get_reactor(WListenerFdRef athis)
{
    return athis->runloop;
}
int WListenerFd_get_fd(WListenerFdRef athis)
{
    return athis->fd;
}

void WListenerFd_verify(WListenerFdRef this)
{
    XR_LISTNER_CHECK_TAG(this)

}
