#ifndef c_demo_server_h
#define c_demo_server_h

#include <c_http/socket_functions.h>
#include <c_http/constants.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http//async/types.h>
#include <c_http/common/message.h>
#include <c_http/async/handler.h>

#define DemoServer_TAG "DmSRVR"
#include <c_http/check_tag.h>

#define XR_NBR_WORKERS 1
typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;
typedef struct  DemoServer_s DemoServer, *DemoServerRef;
struct DemoServer_s {
    DECLARE_TAG;
    int                     port;
    socket_handle_t         listening_socket_fd;
    ReactorRef              reactor_ref;
    RtorListenerRef         listening_watcher_ref;
    TcpConnListRef          conn_list_ref;
    ListRef                 handler_list;
    void(*completion_callback)(DemoServerRef, DemoHandlerRef);
};

/**
 * Create a new server object.
 * \param port     The localhost:port on which the server will listen
 * \param handler  A function conforming to XrHandlerFunction (see aio_api/handler.h) which will be called to handle all requests that are parsed successfully.
 * \return
 */
DemoServerRef DemoServer_new(int port);
void DemoServer_free(DemoServerRef this);
void DemoServer_dispose(DemoServerRef* srefptr);
void DemoServer_listen(DemoServerRef server);
void DemoServer_terminate(DemoServerRef this);
#endif