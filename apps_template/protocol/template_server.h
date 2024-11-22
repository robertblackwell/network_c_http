#ifndef c_tmpl_server_h
#define c_tmpl_server_h
#include <http_in_c/tmpl_protocol/tmpl_message.h>
#include <http_in_c/tmpl_protocol/tmpl_connection.h>
#include <http_in_c/tmpl_protocol/tmpl_handler.h>
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/constants.h>
#include <http_in_c/runloop/runloop.h>
#define TmplServer_TAG "DmSRVR"
#include <rbl/check_tag.h>

#define XR_NBR_WORKERS 1
typedef struct TmplHandler_s TmplHandler, *TmplHandlerRef;
typedef struct  TmplServer_s TmplServer, *TmplServerRef;
typedef void(TmplProcessRequestFunction)(TmplHandlerRef, TmplMessageRef, TmplMessageRef);
struct TmplServer_s {
    RBL_DECLARE_TAG;
    int                     port;
    char const *            host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopListenerRef      listening_watcher_ref;
    ListRef                 handler_list;
    TmplProcessRequestFunction* process_request_function;
    void(*completion_callback)(TmplServerRef, TmplHandlerRef);
    RBL_DECLARE_END_TAG;
};

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
TmplServerRef tmpl_server_new(int port, char const * host, int listen_fd, TmplProcessRequestFunction* process_request);
void tmpl_server_init(TmplServerRef sref, int port, char const * host, int listen_fd, TmplProcessRequestFunction* process_request);
void tmpl_server_free(TmplServerRef this);
void tmpl_server_listen(TmplServerRef server);
void tmpl_server_terminate(TmplServerRef this);
#endif