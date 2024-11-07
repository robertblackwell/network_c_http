//
// Created by robert on 11/7/24.
//
#include "asio_listener.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rbl/logger.h>
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
void asio_listener_init(AsioListenerRef this, RunloopRef rl, int socket_fd)
{
    this->on_accept_callback = NULL;
    this->on_accept_callback_arg = NULL;
    this->rl_listener_ref = runloop_listener_new(rl, socket_fd);
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

void asio_listen(AsioListenerRef this, AcceptCallback on_accept_callback, void* arg)
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
        cb(cb_arg, sock2, errno_saved);
        RBL_LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        cb(cb_arg, sock2, 0);
    }
}
