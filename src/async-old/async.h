#ifndef http_in_c_async_async_h
#define http_in_c_async_async_h

#include <src/runloop/runloop.h>
#include <src/runloop/rl_internal.h>
#include <src/common/list.h>
#include <src/common/iobuffer.h>
#include <src/http_protocol/http_message.h>
#include <src/http_protocol/http_message_parser.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


#define AsyncConnection_TAG "ASYCONN"
#define AsyncHandler_TAG "ASYDLR"
#define AsyncServer_TAG "SYNSRVR"

#include <rbl/check_tag.h>

typedef struct AsyncConnection_s AsyncConnection, *AsyncConnectionRef;
typedef struct AsyncHandler_s AsyncHandler, *AsyncHandlerRef;
typedef struct AsyncServer_s AsyncServer, *AsyncServerRef;

typedef void(*AsyncProcessRequestCompletionFunction)(AsyncHandlerRef handler_ref, MessageRef request_ptr, MessageRef response_ptr);
/**
 * This is where the servers web application is implemented.
 * A function with this signature must be linked into the
 * async_server_app to complete it.
 * For this example this function is found in async_process_request
 *
 * Create a MessageRef response_ptr from the request and other considerations.
 * When done call:
 *
 *      handler_ref->handle_response(handler_ref, request, response)
 *
 * The handler will send the response to the client after maybe adding or modifying
 * some headers.
 *
 * handler_ref->handle_response() is of type AsyncProcessRequestCompletionFunction.
 *
 * Could also call async_handler_handle_response() which is the same function.
 */
typedef void(*AsyncProcessRequestFunction)(AsyncHandlerRef handler_ref, MessageRef request);
//MessageRef process_request(AsyncHandlerRef href, MessageRef request);

/**
 * In the following void* href is an anonymous reference
 * passed to the connection at init time by the parent/sibling object
 * that created the AsyncConnection object
 */
typedef void(*AsyncReadMessageCallback)(void* handler_ptr, MessageRef, int status);
typedef void(*AsyncWriteMessageCallback)(void* handler_ptr, int status);
/**
 * DC_Close_CB is called by the connection when it is closing down
 */
typedef void(*DC_Close_CB)(void* href);

/**
 * A callback which is invoked when a connection closes and the handler is done
 */
typedef void(*DH_Completion_CB)(void* server_ref, AsyncHandlerRef href);


/**=============================================================================
 * AsyncServer api
 * There is one AsyncServer instance for each thread in a async server app
 * Each AsyncServer can manage more than one connection
=============================================================================*/
struct AsyncServer_s {
    RBL_DECLARE_TAG;
    void(*handler_complete)(AsyncServerRef, AsyncHandlerRef);
    AsyncProcessRequestFunction process_request;
    const char*             host;
    int                     port;
    int                     listening_socket_fd;
    RunloopRef              reactor_ref;
    RunloopListenerRef      listening_watcher_ref;
    /**
     * List of handler servicing client connections
     */
    ListRef                 handler_list;
};
#define call_handler_complete(sref, href) \
do {\
    (sref->handler_complete)(sref, href);\
} while (false);

int async_create_shareable_socket();

AsyncServerRef AsyncServer_new(int port, const char* host, AsyncProcessRequestFunction process_request);
AsyncServerRef AsyncServer_new_with_socket(int port, const char* host, int listen_socket, AsyncProcessRequestFunction process_request);
void AsyncServer_init(AsyncServerRef sref, int port, const char* host, AsyncProcessRequestFunction process_request);
void AsyncServer_init_with_socket(AsyncServerRef sref, int port, const char* host, int listen_socket, AsyncProcessRequestFunction process_request);
void AsyncServer_free(AsyncServerRef this);
void AsyncServer_dispose(AsyncServerRef* srefptr);
void AsyncServer_start(AsyncServerRef sref);
void AsyncServer_terminate(AsyncServerRef this);


/**=============================================================================
 * AsyncConnection  api
 * There is one instance of AsyncConnection for every AsyncHandler.
 * The AsyncConnection instance hides the read/write/parser state machines
 * from the handler.
=============================================================================*/
typedef struct AsyncConnection_s {
    RBL_DECLARE_TAG;
    RunloopRef      reactor_ref;
    AsyncHandlerRef handler_ref;
    int             socket;
    RunloopStreamRef   socket_stream_ref;
    IOBufferRef     active_input_buffer_ref;
    IOBufferRef     active_output_buffer_ref;
    http_parser_t*  http_parser_ptr;
    MessageRef      input_message_ptr;
    MessageRef      scratch_request;
    MessageRef      output_message_ptr;
    int             read_state;
    int             write_state;
    int             read_buffer_size;
    bool            cleanup_done_flag;
    bool            readside_posted;
    bool            writeside_posted;

} AsyncConnection, *AsyncConnectionRef;

AsyncConnectionRef async_connection_new(
        int                 socket,
        RunloopRef          reactor_ref,
        AsyncHandlerRef     handler_ref
);
void async_connection_init(
        AsyncConnectionRef this,
        int                 socket,
        RunloopRef          reactor_ref,
        AsyncHandlerRef      handler_ref
);

void async_connection_free(AsyncConnectionRef this);
void async_connection_destroy(AsyncConnectionRef cref);
void async_connection_amonymous_dispose(void* p);
void async_connection_read(AsyncConnectionRef connection_ref); //, void(*on_read_message_cb)(void* href, MessageRef, int status));
void async_connection_write(AsyncConnectionRef connection_ref, MessageRef); //, void(*on_write_message_cb)(void* href, int status));


/**=============================================================================
 * AsyncHandler  api
 * Within an AsyncServer there is one instance of AsyncHandler for each
 * active connection. These are created when an accept returns and
 * destroyed when a connection closes.
=============================================================================*/
typedef struct AsyncHandler_s {
    RBL_DECLARE_TAG;

    void(*handle_request)(AsyncHandlerRef, MessageRef);
    void(*handle_response)(AsyncHandlerRef, MessageRef request, MessageRef response);
    void(*handle_write_complete)(AsyncHandlerRef);
    void(*handle_close_connection)(AsyncHandlerRef);

//    void(*handle_reader_stopped)(AsyncHandlerRef);
//    void(*handle_io_error)(AsyncHandlerRef);
//    void(*handle_write_failed)(AsyncHandlerRef);

    AsyncConnectionRef  async_connection_ref;
    AsyncServerRef      server_ref;
    ListRef             input_list; // List of MessageRef - requests
    ListRef             output_list; // List of MessageRef - responses
//    MessageRef          active_response;

} AsyncHandler, *AsyncHandlerRef;

AsyncHandlerRef async_handler_new(
        int socket,
        RunloopRef reactor_ref,
        AsyncServerRef sref
);
void async_handler_init(
        AsyncHandlerRef this, int socket,
        RunloopRef reactor_ref,
        AsyncServerRef sref
);
void async_handler_handle_response(AsyncHandlerRef href, MessageRef request_ptr, MessageRef response_ptr);
void async_handler_free(AsyncHandlerRef this);
void async_handler_anonymous_dispose(void** p);
inline RunloopRef async_handler_reactor_ref(AsyncHandlerRef handler_ref)
{
    return handler_ref->async_connection_ref->reactor_ref;
}
inline int async_handler_socket(AsyncHandlerRef handler_ref)
{
    return handler_ref->async_connection_ref->socket;
}
int async_handler_threadid(AsyncHandlerRef handler_ref);
#endif