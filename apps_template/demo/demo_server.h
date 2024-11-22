#ifndef c_demo_server_h
#define c_demo_server_h
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_connection.h>
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/constants.h>
#include <http_in_c/runloop/runloop.h>
#define DemoServer_TAG "DmSRVR"
#include <rbl/check_tag.h>

#define XR_NBR_WORKERS 1
typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;
typedef struct  DemoServer_s DemoServer, *DemoServerRef;
typedef void(DemoProcessRequestFunction)(DemoHandlerRef, DemoMessageRef, DemoMessageRef);
struct DemoServer_s {
    RBL_DECLARE_TAG;
    int                     port;
    char const *            host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopListenerRef      listening_watcher_ref;
    ListRef                 handler_list;
    DemoProcessRequestFunction* process_request_function;
    void(*completion_callback)(DemoServerRef, DemoHandlerRef);
    RBL_DECLARE_END_TAG;
};

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
DemoServerRef demo_server_new(int port, char const * host, int listen_fd, DemoProcessRequestFunction* process_request);
void demo_server_init(DemoServerRef sref, int port, char const * host, int listen_fd, DemoProcessRequestFunction* process_request);
void demo_server_free(DemoServerRef this);
void demo_server_listen(DemoServerRef server);
void demo_server_terminate(DemoServerRef this);
#endif