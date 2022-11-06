#ifndef c_http_xr_server_h
#define c_http_xr_server_h

#include <c_http/socket_functions.h>
#include <c_http/constants.h>
#include <c_http/runloop/types.h>
#include <c_http//async/types.h>
#include <c_http/common/message.h>
#include <c_http/async/handler.h>

#define XR_NBR_WORKERS 1
struct AsyncServer_s {
    int                     port;
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    ReactorRef            reactor_ref;
    WListenerFdRef          listening_watcher_ref;
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