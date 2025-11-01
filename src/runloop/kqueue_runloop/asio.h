#ifndef h_kq_asio_h
#define h_kq_asio_h
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/asio_forward.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Asio Stream
///////////////////////////////////////////////////////////////////////////////////////////////////////////
AsioStreamRef asio_stream_new(RunloopRef runloop_ref, int socket);
void asio_stream_free(AsioStreamRef asio_this);
void asio_stream_init(AsioStreamRef asio_this, RunloopRef runloop_ref, int fd);
void asio_stream_deinit(AsioStreamRef cref);
void asio_stream_read(AsioStreamRef stream_ref, void* buffer, long max_length, AsioReadcallback cb, void*  arg);
void asio_stream_write(AsioStreamRef stream_ref, void* buffer, long length, AsioWritecallback cb, void*  arg);
void asio_stream_close(AsioStreamRef cref);
RunloopRef asio_stream_get_runloop(AsioStreamRef asio_stream_ref);
int asio_stream_get_fd(AsioStreamRef asio_stream_ref);
RunloopStreamRef asio_stream_get_runloop_stream(AsioStreamRef asio_stream_ref);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Asio Listener
///////////////////////////////////////////////////////////////////////////////////////////////////////////
AsioListenerRef asio_listener_new_from_port_host(RunloopRef rlref, int port, const char* host);
AsioListenerRef asio_listener_new(RunloopRef rlref, int socket_fd);
void asio_listener_init(AsioListenerRef asio_this, RunloopRef rl, int socket_fd);
void asio_listen_init_from_port_host(AsioListenerRef asio_this, int port , const char* host);
void asio_listener_deinit(AsioListenerRef asio_this);
void asio_listener_free(AsioListenerRef asio_this);

/**
 * This function will issue an accept() call when the underlying file descriptor is ready for such a call.
 *
 * If there are multiple threads and/or processes listening to sockets with the same port/host combination
 * the OS will only notify one such thread/process for each available client connection.
 *
 * This is a single short call in that once the on_accept_cb is called the accept() function must be called
 * again to accept() subsequent client connections.
 *
 * @param alistener_ref Pointer to a AsioListenRef
 * @param on_accept_cb  callback function
 * @param arg           pointer to a user defined object providing context to the callback
 */
void asio_accept(AsioListenerRef alistener_ref, void(on_accept_cb)(void* arg, int accepted_fd, int error), void* arg);


#endif