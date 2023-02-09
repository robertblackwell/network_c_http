#ifndef c_http_xr_server_h
#define c_http_xr_server_h

#include <http_in_c/socket_functions.h>
#include <http_in_c/constants.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c//async/types.h>
#include <http_in_c/http/message.h>
#include <http_in_c/async/handler.h>

#define XR_NBR_WORKERS 1
struct AsyncServer_s {
    int                     port;
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    ReactorRef            reactor_ref;
    RtorListenerRef          listening_watcher_ref;
    TcpConnListRef           conn_list_ref;
};
typedef struct  AsyncServer_s AsyncServer, *AsyncServerRef;

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
AsyncServerRef AsyncServer_new(int port);
void AsyncServer_dispose(AsyncServerRef* srefptr);
void AsyncServer_listen(AsyncServerRef server);
void AsyncServer_terminate(AsyncServerRef this);
#endif