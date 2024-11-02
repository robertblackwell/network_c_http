#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>
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
static void handler(RunloopWatcherRef watcher, uint64_t event)
{
    RunloopListenerRef listener_ref = (RunloopListenerRef)watcher;
    if(listener_ref->listen_evhandler) {
        listener_ref->listen_evhandler(listener_ref,  event);
    }
}
static void anonymous_free(RunloopWatcherRef p)
{
    runloop_listener_free((RunloopListenerRef) p);
}
void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd)
{
    RBL_SET_TAG(WListenerFd_TAG, athis);
    athis->type = RUNLOOP_WATCHER_LISTENER;
    athis->fd = fd;
    athis->runloop = runloop;
    athis->free = &anonymous_free;
    athis->handler = &handler;
    athis->context = athis;
    athis->listen_arg = NULL;
    athis->listen_evhandler = NULL;
}
RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd)
{
    RunloopListenerRef this = malloc(sizeof(RunloopListener));
    runloop_listener_init(this, runloop, fd);
    return this;
}
void runloop_listener_free(RunloopListenerRef athis)
{
    runloop_listener_verify(athis);
    close(athis->fd);
    free((void*)athis);
}
void runloop_listener_register(RunloopListenerRef athis, ListenerEventHandler event_handler, void* arg)
{
    runloop_listener_verify(athis);
    athis->handler = &handler;
    athis->context = athis;
    if( event_handler != NULL) {
        athis->listen_evhandler = event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }

    /**
     * NOTE the EPOLLEXCLUSIVE - prevents the thundering herd problem. Defaults to level triggered
     */
    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherRef) (athis));

    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_deregister(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    runloop_deregister(athis->runloop, athis->fd);
}
void runloop_listener_arm(RunloopListenerRef athis, ListenerEventHandler fd_event_handler, void* arg)
{
    LISTNER_CHECK_TAG(athis)
    if(fd_event_handler != NULL) {
        athis->listen_evhandler = fd_event_handler;
    }
    if (arg != NULL) {
        athis->listen_arg = arg;
    }
    uint32_t interest = EPOLLIN; // | EPOLLEXCLUSIVE ;

    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherRef) athis);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void WListenerFd_disarm(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    int res = runloop_reregister(athis->runloop, athis->fd, 0L, (RunloopWatcherRef) athis);
    assert(res == 0);
}
RunloopRef runloop_listener_get_reactor(RunloopListenerRef athis)
{
    return athis->runloop;
}
int runloop_listener_get_fd(RunloopListenerRef this)
{
    return this->fd;
}

void runloop_listener_verify(RunloopListenerRef r)
{
    LISTNER_CHECK_TAG(r)

}
