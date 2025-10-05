#ifndef C_HTTP_runloop_W_LISTENER_H
#define C_HTTP_runloop_W_LISTENER_H
#include "runloop.h"
/**
 * This function must be called on the same thread that is going to call
 * runloop_run(rlref)
 *
 * @param rlref            RunloopRef for the runloop being use for the active thread and file descriptor
 * @param port int         a port nuimber to listen on.
 * @param host const char* a host name or ip address
 * @return  AsiListenerRef
 * @throws if something fails
 */
AsioListenerRef asio_listener_new_from_port_host(RunloopRef rlref, int port, const char* host);
/**
 * This function must be called on the same thread that is going to call
 * runloop_run(rlref)
 *
 * @param rlref         RunloopRef for the runloop being use for the active thread and file descriptor
 * @param socket_fd     a valid file descriptor for a TCP server socket that is non-blocking.
 *                      A server socket is one that has been the subject of a bind() and listen()
 *                      calls.
 *                      Usually acquired by a call to `create_listen_socket()`
 * @return  AsiListenerRef
 * @throws if something fails
 */
AsioListenerRef asio_listener_new(RunloopRef rlref, int socket_fd);
/**
 * This function must be called on the same thread that is going to call
 * runloop_run(rlref)
 *
 * @param this      a pointer to a variable of type AsioListener
 * @param rl        RunloopRef
 * @param socket_fd Same requirements if asio_listener_new()
 */
void asio_listener_init(AsioListenerRef this, RunloopRef rl, int socket_fd);

/**
 * This function must be called on the same thread that is going to call
 * runloop_run(rlref) rlref is the first param
 *
 * @param this          a pointer to a variable of type AsioListener
 * @param rlref         RunloopRef for the runloop being use for the active thread and file descriptor
 * @param socket_fd     a valid file descriptor for a TCP server socket that is non-blocking.
 *                      A server socket is one that has been the subject of a bind() and listen()
 *                      call.
 *                      Usually acquired by a call to `create_listen_socket()`
 * @return  AsiListenerRef
 * @throws if something fails
 */
void asio_listen_init_from_port_host(AsioListenerRef this, int port , const char* host);

void asio_listener_deinit(AsioListenerRef this);
void asio_listener_free(AsioListenerRef this);

/**
 * This function will issue an accept() call when the underlying file descriptor is ready for such a call.
 *
 * If there are multiple threads and/or processes listening to sockets with the same port/host combination
 * the Linux OS will only notify one such thread/process for each available client connection.
 *
 * This is a single short call in that once the on_accept_cb is called the accept() function must be called
 * again to accept() subsequent client connections.
 *
 * @param alistener_ref Pointer to a AsioListenRef
 * @param on_accept_cb  callback function
 * @param arg           pointer to a user defined object providing context to the callback
 */
void asio_accept(AsioListenerRef alistener_ref, void(on_accept_cb)(void* arg, int accepted_fd, int error), void* arg);

RunloopListenerRef runloop_listener_new(RunloopRef runloop, int fd);
void runloop_listener_free(RunloopListenerRef athis);

void runloop_listener_init(RunloopListenerRef athis, RunloopRef runloop, int fd);
void runloop_listener_deinit(RunloopListenerRef athis);

void runloop_listener_register(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_deregister(RunloopListenerRef athis);
void runloop_listener_arm(RunloopListenerRef athis, PostableFunction postable, void* postable_arg);
void runloop_listener_disarm(RunloopListenerRef athis);
void runloop_listener_verify(RunloopListenerRef r);
RunloopRef runloop_listener_get_runloop(RunloopListenerRef athis);
int runloop_listener_get_fd(RunloopListenerRef this);
/** @} */
#endif //C_HTTP_runloop_W_LISTENER_H
