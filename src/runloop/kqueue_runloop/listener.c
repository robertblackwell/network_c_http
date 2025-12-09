#include "runloop_internal.h"
#include <common/socket_functions.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rbl/logger.h>

/**
 * Called whenever an fd associated with an Listener receives an fd event.
 * Should dispatch the read event handler on read ready.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopEventBaseRef lrwatcher, uint16_t event, uint16_t flags, void* data)
{
    RunloopListenerRef listener_ref = (RunloopListenerRef)lrwatcher;
    LISTNER_CHECK_TAG(listener_ref)
    LISTNER_CHECK_END_TAG(listener_ref)
    RBL_LOG_FMT("listener handler")
    if(listener_ref->listen_postable) {
        /**
         * This should be posted not called
         */
        listener_ref->listen_postable(listener_ref->runloop,  listener_ref->listen_postable_arg);
    }
}
static void anonymous_free(RunloopListenerRef p)
{
    LISTNER_CHECK_TAG((RunloopListenerRef)p)
    LISTNER_CHECK_END_TAG((RunloopListenerRef)p)
    runloop_listener_free((RunloopListenerRef) p);
}

void runloop_listener_init(RunloopListenerRef listener, RunloopRef runloop, int fd)
{
    LISTNER_SET_TAG(listener);
    LISTNER_SET_END_TAG(listener);
    listener->type = RUNLOOP_EVENT_LISTENER;
    listener->runloop = runloop;
    listener->handler = &handler;
    listener->context = listener;
    listener->fd = fd;
    listener->listen_postable_arg = NULL;
    listener->listen_postable = NULL;
}
void runloop_listener_deinit(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    // does not own any dynamic objects
}
RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd)
{
    RunloopListenerRef listener = runloop_event_allocate(runloop, sizeof(RunloopListener));
    runloop_listener_init(listener, runloop, fd);
    return listener;
}
void runloop_listener_free(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    runloop_listener_verify(listener);
    close(listener->fd);
    runloop_event_free(listener->runloop, listener);
}
void runloop_listener_register(RunloopListenerRef listener, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    runloop_listener_verify(listener);
    listener->handler = &handler;
    listener->context = listener;
    if( postable != NULL) {
        listener->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        listener->listen_postable_arg = postable_arg;
    }
    int res = kqh_listener_register(listener);
    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}

void runloop_listener_deregister(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    kqh_listener_cancel(listener);
}
void runloop_listener_arm(RunloopListenerRef listener, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    if(postable != NULL) {
        listener->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        listener->listen_postable_arg = postable_arg;
    }
    int res = kqh_listener_register(listener);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_rearm(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    int res = kqh_listener_register(listener);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_disarm(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    int res = kqh_listener_pause(listener);
    assert(res == 0);
}
RunloopRef runloop_listener_get_runloop(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    return listener->runloop;
}
int runloop_listener_get_fd(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
    return listener->fd;
}

void runloop_listener_verify(RunloopListenerRef listener)
{
    LISTNER_CHECK_TAG(listener)
    LISTNER_CHECK_END_TAG(listener)
}
