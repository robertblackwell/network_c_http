#include "runloop_internal.h"
#include <common/socket_functions.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rbl/logger.h>
#include "epoll_helper.h"

/**
 * Called whenever an fd associated with an WListener receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopWatcherBaseRef watcher, uint64_t event)
{
    RunloopListenerRef listener_ref = (RunloopListenerRef)watcher;
    LISTNER_CHECK_TAG(listener_ref)
    LISTNER_CHECK_END_TAG(listener_ref)
    if(listener_ref->listen_postable) {
        /**
         * This should be posted not called
         */
        listener_ref->listen_postable(listener_ref->runloop,  listener_ref->listen_postable_arg);
    }
}
static void anonymous_free(RunloopWatcherBaseRef p)
{
    LISTNER_CHECK_TAG((RunloopListenerRef)p)
    LISTNER_CHECK_END_TAG((RunloopListenerRef)p)
    runloop_listener_free((RunloopListenerRef) p);
}

void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd)
{
    LISTNER_SET_TAG(athis);
    LISTNER_SET_END_TAG(athis)
    athis->type = RUNLOOP_WATCHER_LISTENER;
    athis->fd = fd;
    athis->runloop = runloop;
    athis->free = &anonymous_free;
    athis->handler = &handler;
    athis->context = athis;
    athis->listen_postable_arg = NULL;
    athis->listen_postable = NULL;
}
void runloop_listener_deinit(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    // does not own any dynamic objects
}
RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd)
{
    RunloopListenerRef this = rl_event_allocate(runloop, sizeof(RunloopListener));
    runloop_listener_init(this, runloop, fd);
    return this;
}
void runloop_listener_free(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    runloop_listener_verify(athis);
    close(athis->fd);
    rl_event_free(athis->runloop, athis);
}
void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    runloop_listener_verify(athis);
    athis->handler = &handler;
    athis->context = athis;
    if( postable != NULL) {
        athis->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        athis->listen_postable_arg = postable_arg;
    }
    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    eph_add(athis->runloop->epoll_fd, athis->fd, interest, (athis));
}
void runloop_listener_deregister(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    eph_del(athis->runloop->epoll_fd, athis->fd, (uint32_t)0, (athis));
}
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    if(postable != NULL) {
        athis->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        athis->listen_postable_arg = postable_arg;
    }
    assert(0); // something is wrong here
    uint32_t interest = EPOLLIN; // | EPOLLEXCLUSIVE ;

    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, (RunloopWatcherBaseRef) athis);
}
void runloop_listener_disarm(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    uint32_t interest = 0;
    eph_mod(athis->runloop->epoll_fd, athis->fd, interest, (RunloopWatcherBaseRef) athis);
}
RunloopRef runloop_listener_get_runloop(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    return athis->runloop;
}
int runloop_listener_get_fd(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    return athis->fd;
}

void runloop_listener_verify(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
}
