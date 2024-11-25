#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>
#include <http_in_c/common/socket_functions.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
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
static void handler(RunloopWatcherBaseRef watcher, uint64_t event)
{
    RunloopListenerRef listener_ref = (RunloopListenerRef)watcher;
    if(listener_ref->listen_postable) {
        /**
         * This should be posted not called
         */
        listener_ref->listen_postable(listener_ref->runloop,  listener_ref->listen_postable_arg);
    }
}
static void anonymous_free(RunloopWatcherBaseRef p)
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
    athis->listen_postable_arg = NULL;
    athis->listen_postable = NULL;
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
void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg)
{
    runloop_listener_verify(athis);
    athis->handler = &handler;
    athis->context = athis;
    if( postable != NULL) {
        athis->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        athis->listen_postable_arg = postable_arg;
    }

    /**
     * NOTE the EPOLLEXCLUSIVE - prevents the thundering herd problem. Defaults to level triggered
     */
    uint32_t interest =  EPOLLIN | EPOLLEXCLUSIVE;
    int res = runloop_register(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) (athis));

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
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg)
{
    LISTNER_CHECK_TAG(athis)
    if(postable != NULL) {
        athis->listen_postable = postable;
    }
    if (postable_arg != NULL) {
        athis->listen_postable_arg = postable_arg;
    }
    uint32_t interest = EPOLLIN; // | EPOLLEXCLUSIVE ;

    int res = runloop_reregister(athis->runloop, athis->fd, interest, (RunloopWatcherBaseRef) athis);
    if(res != 0) {
        printf("arm status : %d errno: %d \n", res, errno);
    }
    assert(res == 0);
}
void runloop_listener_disarm(RunloopListenerRef athis)
{
    LISTNER_CHECK_TAG(athis)
    int res = runloop_reregister(athis->runloop, athis->fd, 0L, (RunloopWatcherBaseRef) athis);
    assert(res == 0);
}
RunloopRef runloop_listener_get_runloop(RunloopListenerRef athis)
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

/****************************************************************************************************************
 * start of asio_listener code
 *****************************************************************************************************************/
static void on_listening_postable(RunloopRef rl, void* asio_listener_arg);

typedef struct AsioListener_s {
    RunloopListenerRef rl_listener_ref;
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
    printf("listening_hander \n");
    AsioListenerRef asio_listener_ref  = asio_listener_arg;
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
