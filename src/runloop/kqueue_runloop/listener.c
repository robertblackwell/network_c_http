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
 * Called whenever an fd associated with an WListener receives an fd event.
 * Should dispatch the read_evhandler and/or write_evhandler depending on whether those
 * events (read events and write events) are armed.
 * @param ctx       void*
 * @param fd        int
 * @param event     uint64_t
 */
static void handler(RunloopEventRef lrevent, uint16_t event, uint16_t flags)
{
    RunloopEventRef listener_ref = (RunloopEventRef)lrevent;
    LISTNER_CHECK_TAG(listener_ref)
    LISTNER_CHECK_END_TAG(listener_ref)
    RBL_LOG_FMT("listener handler")
    if(listener_ref->listener.listen_postable) {
        /**
         * This should be posted not called
         */
        listener_ref->listener.listen_postable(listener_ref->runloop,  listener_ref->listener.listen_postable_arg);
    }
}
static void anonymous_free(RunloopEventRef p)
{
    LISTNER_CHECK_TAG((RunloopListenerRef)p)
    LISTNER_CHECK_END_TAG((RunloopListenerRef)p)
    runloop_listener_free((RunloopEventRef) p);
}

void runloop_listener_init(RunloopEventRef lrevent, RunloopRef runloop, int fd)
{
    LISTNER_SET_TAG(lrevent);
    LISTNER_SET_END_TAG(lrevent)
    lrevent->type = RUNLOOP_WATCHER_LISTENER;
    lrevent->runloop = runloop;
    lrevent->free = &anonymous_free;
    lrevent->handler = &handler;
    lrevent->context = lrevent;
    lrevent->listener.fd = fd;
    lrevent->listener.listen_postable_arg = NULL;
    lrevent->listener.listen_postable = NULL;
}
void runloop_listener_deinit(RunloopEventRef lrevent)
{
    LISTNER_CHECK_TAG(lrevent)
    LISTNER_CHECK_END_TAG(lrevent)
    // does not own any dynamic objects
}
RunloopEventRef runloop_listener_new(RunloopRef runloop, int fd)
{
    RunloopEventRef this = event_table_get_entry(runloop->event_table);
    int x = event_table_number_in_use(runloop->event_table);
    runloop_listener_init(this, runloop, fd);
    return this;
}
void runloop_listener_free(RunloopEventRef rlevent)
{
    LISTNER_CHECK_TAG(rlevent)
    LISTNER_CHECK_END_TAG(rlevent)
    runloop_listener_verify(rlevent);
    close(rlevent->listener.fd);
    event_table_release_entry(rlevent->runloop->event_table, rlevent);
}
void runloop_listener_register(RunloopEventRef lrevent, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(lrevent)
    LISTNER_CHECK_END_TAG(lrevent)
    runloop_listener_verify(lrevent);
    lrevent->handler = &handler;
    lrevent->context = lrevent;
    if( postable != NULL) {
        lrevent->listener.listen_postable = postable;
    }
    if (postable_arg != NULL) {
        lrevent->listener.listen_postable_arg = postable_arg;
    }
    int res = kqh_listener_register(lrevent);
    if(res != 0) {
        printf("register status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}

void runloop_listener_deregister(RunloopEventRef lrevent)
{
    LISTNER_CHECK_TAG(lrevent)
    LISTNER_CHECK_END_TAG(lrevent)
    kqh_listener_cancel(lrevent);
}
void runloop_listener_arm(RunloopEventRef lrevent, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(lrevent)
    LISTNER_CHECK_END_TAG(lrevent)
    if(postable != NULL) {
        lrevent->listener.listen_postable = postable;
    }
    if (postable_arg != NULL) {
        lrevent->listener.listen_postable_arg = postable_arg;
    }
    int res = kqh_listener_register(lrevent);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_rearm(RunloopEventRef lrevent)
{
    LISTNER_CHECK_TAG(lrevent)
    LISTNER_CHECK_END_TAG(lrevent)
    int res = kqh_listener_register(lrevent);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_disarm(RunloopEventRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    int res = kqh_listener_pause(athis);
    assert(res == 0);
}
RunloopRef runloop_listener_get_runloop(RunloopEventRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    return athis->runloop;
}
int runloop_listener_get_fd(RunloopEventRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
    return athis->listener.fd;
}

void runloop_listener_verify(RunloopEventRef athis)
{
    LISTNER_CHECK_TAG(athis)
    LISTNER_CHECK_END_TAG(athis)
}
#if 0
/****************************************************************************************************************
 * start of asio_listener code
 *****************************************************************************************************************/
static void on_listening_postable(RunloopRef rl, void* asio_listener_arg);

typedef struct AsioListener_s {
    RunloopEventRef     rl_listener_ref;
    AcceptCallback      on_accept_callback;
    void*               on_accept_callback_arg;
} AsioListener, *AsioListenerRef;

AsioListenerRef asio_listener_new(RunloopRef rl, int socket_fd)
{
    AsioListenerRef this = malloc(sizeof(AsioListener));
    asio_listener_init(this, rl, socket_fd);
    return this;
}
AsioListenerRef asio_listener_new_from_port_host(RunloopRef rl, int port, const char* host)
{
    int fd = create_listener_socket(port, host);
    socket_set_non_blocking(fd);
    AsioListenerRef asio_listener_ref = asio_listener_new(rl, fd);
    return asio_listener_ref;
}
void asio_listener_init(AsioListenerRef this, RunloopRef rl, int socket_fd)
{
    this->on_accept_callback = NULL;
    this->on_accept_callback_arg = NULL;
    this->rl_listener_ref = runloop_listener_new(rl, socket_fd);
}
void asio_listener_init_from_port_host(AsioListenerRef this,  RunloopRef rl, int port, const char* host)
{
    int fd = create_listener_socket(port, host);
    socket_set_non_blocking(fd);
    asio_listener_init(this, rl, fd);
}
void asio_listener_deinit(AsioListenerRef this)
{
    this->on_accept_callback = NULL;
    this->on_accept_callback_arg = NULL;
    runloop_listener_free(this->rl_listener_ref);
    this->rl_listener_ref = NULL;
}
void asio_listener_free(AsioListenerRef this)
{
    asio_listener_deinit(this);
    free(this);
}

void asio_accept(AsioListenerRef this, AcceptCallback on_accept_callback, void* arg)
{
    this->on_accept_callback = on_accept_callback;
    this->on_accept_callback_arg = arg;
    runloop_listener_register(this->rl_listener_ref, on_listening_postable, this);
}
static void on_listening_postable(RunloopRef rl, void* asio_listener_arg)
{
    AsioListenerRef asio_listener_ref  = asio_listener_arg;
    printf("listening_handler fd: %d\n", asio_listener_ref->rl_listener_ref->listener.fd);
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    int fd = runloop_listener_get_fd(asio_listener_ref->rl_listener_ref);
    AcceptCallback cb = asio_listener_ref->on_accept_callback;
    void* cb_arg = asio_listener_ref->on_accept_callback_arg;

    int sock2 = accept(fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        int errno_saved = errno;
        asio_listener_ref->on_accept_callback = NULL;
        asio_listener_ref->on_accept_callback_arg = NULL;
        runloop_listener_disarm(asio_listener_ref->rl_listener_ref);
        cb(cb_arg, sock2, errno_saved);
        RBL_LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        asio_listener_ref->on_accept_callback = NULL;
        asio_listener_ref->on_accept_callback_arg = NULL;
        runloop_listener_deregister(asio_listener_ref->rl_listener_ref);
        cb(cb_arg, sock2, 0);
    }
}
#endif