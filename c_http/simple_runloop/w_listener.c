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
static void handler(RtorWatcherRef watcher, uint64_t event)
{
    RtorListenerRef listener_ref = (RtorListenerRef)watcher;
    if(listener_ref->listen_evhandler) {
        listener_ref->listen_evhandler(listener_ref,  event);
    }
}
static void anonymous_free(RtorWatcherRef p)
{
    rtor_Listener_free((RtorListenerRef) p);
}
void WListenerFd_init(RtorListenerRef athis, ReactorRef runloop, int fd)
{
    XR_LISTNER_SET_TAG(athis);
    athis->type = XR_WATCHER_LISTENER;
    athis->fd = fd;
    athis->runloop = runloop;
    athis->free = &anonymous_free;
    athis->handler = &handler;
    athis->listen_arg = NULL;
    athis->listen_evhandler = NULL;
}
RtorListenerRef rtor_listener_new(ReactorRef runloop, int fd)
{
    RtorListenerRef this = malloc(sizeof(RtorListener));
    WListenerFd_init(this, runloop, fd);
    return this;
}
void rtor_Listener_free(RtorListenerRef athis)
{
    rtor_listener_verify(athis);
    close(athis->fd);
    free((void*)athis);
}
void rtor_listener_register(RtorListenerRef athis, ListenerEventHandler event_handler, void* arg)
{
    rtor_listener_verify(athis);
    if( event_handler != NULL) {
        athis->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }

    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    int res = rtor_register(athis->runloop, athis->fd, interest, (RtorWatcherRef) (athis));
    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void rtor_listener_deregister(RtorListenerRef athis)
{
    XR_LISTNER_CHECK_TAG(athis)
    rtor_delete(athis->runloop, athis->fd);
}
void rtor_listener_arm(RtorListenerRef athis, ListenerEventHandler fd_event_handler, void* arg)
{
    XR_LISTNER_CHECK_TAG(athis)
    if(fd_event_handler != NULL) {
        athis->listen_evhandler = fd_event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }
    uint32_t interest = EPOLLIN | EPOLLEXCLUSIVE ;

    int res = rtor_reregister(athis->runloop, athis->fd, interest, (RtorWatcherRef) athis);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void WListenerFd_disarm(RtorListenerRef athis)
{
    XR_LISTNER_CHECK_TAG(athis)
    int res = rtor_reregister(athis->runloop, athis->fd, 0L, (RtorWatcherRef) athis);
    assert(res == 0);
}
ReactorRef WListenerFd_get_reactor(RtorListenerRef athis)
{
    return athis->runloop;
}
int rtor_listener_get_fd(RtorListenerRef this)
{
    return this->fd;
}

void rtor_listener_verify(RtorListenerRef r)
{
    XR_LISTNER_CHECK_TAG(r)

}
