#ifndef c_http_server_h
#define c_http_server_h
#include <src/http_protocol/http_message.h>
#include <src/http_protocol/http_connection.h>
#include <src/http_protocol/http_handler.h>
#include <src/common/socket_functions.h>
#include <src/constants.h>
#include <src/runloop/runloop.h>
#define HttpServer_TAG "DmSRVR"
#include <rbl/check_tag.h>

#define XR_NBR_WORKERS 1
typedef struct HttpHandler_s HttpHandler, *HttpHandlerRef;
typedef struct  HttpServer_s HttpServer, *HttpServerRef;
typedef void(HttpProcessRequestFunction)(HttpHandlerRef, HttpMessageRef, HttpMessageRef);
struct HttpServer_s {
    RBL_DECLARE_TAG;
    int                     port;
    char const *            host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopListenerRef      listening_watcher_ref;
    ListRef                 handler_list;
    HttpProcessRequestFunction* process_request_function;
    void(*completion_callback)(HttpServerRef, HttpHandlerRef);
    RBL_DECLARE_END_TAG;
};

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
HttpServerRef http_server_new(int port, char const * host, int listen_fd, HttpProcessRequestFunction* process_request);
void http_server_init(HttpServerRef sref, int port, char const * host, int listen_fd, HttpProcessRequestFunction* process_request);
void http_server_free(HttpServerRef this);
void http_server_listen(HttpServerRef server);
void http_server_terminate(HttpServerRef this);
#endif