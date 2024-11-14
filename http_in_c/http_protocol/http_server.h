#ifndef c_http_server_h
#define c_http_server_h
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/http_protocol/http_connection.h>
#include <http_in_c/http_protocol/http_handler.h>
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/constants.h>
#include <http_in_c/runloop/runloop.h>

#include <http_in_c/http/message.h>

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
HttpServerRef HttpServer_new(int port, char const * host, int listen_fd, HttpProcessRequestFunction* process_request);
void HttpServer_init(HttpServerRef sref, int port, char const * host, int listen_fd, HttpProcessRequestFunction* process_request);
void HttpServer_free(HttpServerRef this);
void HttpServer_listen(HttpServerRef server);
void HttpServer_terminate(HttpServerRef this);
#endif