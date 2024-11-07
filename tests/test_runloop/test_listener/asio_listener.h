#ifndef C_HTTP_ASIO_LISTENER_H
#define C_HTTP_ASIO_LISTENER_H
#include <http_in_c/runloop/runloop.h>

typedef void(*AcceptCallback)(void* arg, int accepted_fd, int errno);

struct AsioListener_s;
typedef struct AsioListener_s AsioListener, *AsioListenerRef;

AsioListenerRef asio_listener_new(RunloopRef rlref, int socket_fd);
void asio_listener_init(AsioListenerRef this, RunloopRef rl, int socket_fd);
void asio_listener_deinit(AsioListenerRef this);
void asio_listener_free(AsioListenerRef this);
void asio_listen(AsioListenerRef alistener_ref, void(on_accept_cb)(void* arg, int accepted_fd, int error), void* arg);

#endif //C_HTTP_ASIO_LISTENER_H
