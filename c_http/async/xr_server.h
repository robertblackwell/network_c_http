#ifndef c_http_xr_server_h
#define c_http_xr_server_h

#include <c_http/constants.h>
#include <c_http/runloop/evfd_queue.h>
#include <c_http/common/message.h>
#include <c_http/socket_functions.h>
#include <c_http/async/handler.h>

#define XR_NBR_WORKERS 1
struct XrServer_s {
    int                     port;
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    XrReactorRef            reactor_ref;
    WListenerRef            listening_watcher_ref;
    XrConnListRef           conn_list_ref;
};
typedef struct  XrServer_s XrServer, *XrServerRef;

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
XrServerRef XrServer_new(int port);
void XrServer_dispose(XrServerRef* srefptr);
void XrServer_listen(XrServerRef server);
void XrServer_terminate(XrServerRef this);
#endif