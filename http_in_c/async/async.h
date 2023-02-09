#ifndef http_in_c_async_async_h
#define http_in_c_async_async_h

#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>
#include <http_in_c/http/message.h>
#include <http_in_c/http/parser.h>

#define AsyncConnection_TAG "ASYCONN"
#define AsyncHandler_TAG "ASYDLR"
#define AsyncServer_TAG "SYNSRVR"

#include <http_in_c/check_tag.h>

enum AsyncConnectionErrCode {
    AsyncConnectionErrCode_is_closed = -31,
    AsyncConnectionErrCode_io_error = -32,
    AsyncConnectionErrCode_parse_error = -33
};

typedef struct AsyncConnection_s AsyncConnection, *AsyncConnectionRef;
typedef struct AsyncHandler_s AsyncHandler, *AsyncHandlerRef;
typedef struct  AsyncServer_s AsyncServer, *AsyncServerRef;

/**
 * This is where the servers web application is implemented.
 */
MessageRef process_request(AsyncHandlerRef href, MessageRef request);

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
    DECLARE_TAG;
    void(*handler_complete)(AsyncServerRef, AsyncHandlerRef);
    int                     port;
    int                     listening_socket_fd;
    ReactorRef              reactor_ref;
    RtorListenerRef         listening_watcher_ref;
    /**
     * List of handler servicing client connections
     */
    ListRef                 handler_list;
};
#define call_handler_complete(sref, href) \
do {\
    (sref->handler_complete)(sref, href);\
} while (false);

AsyncServerRef AsyncServer_new(int port);
void AsyncServer_init(AsyncServerRef sref, int port);
void AsyncServer_free(AsyncServerRef this);
void AsyncServer_dispose(AsyncServerRef* srefptr);
void AsyncServer_listen(AsyncServerRef server);
void AsyncServer_terminate(AsyncServerRef this);


/**=============================================================================
 * AsyncConnection  api
 * There is one instance of AsyncConnection for every AsyncHandler.
 * The AsyncConnection instance hides the read/write/parser state machines
 * from the handler.
=============================================================================*/
typedef struct AsyncConnection_s {
    DECLARE_TAG;
    ReactorRef      reactor_ref;
    int             socket;
    RtorStreamRef   socket_stream_ref;
    AsyncHandlerRef handler_ref;
    IOBufferRef     active_input_buffer_ref;
    IOBufferRef     active_output_buffer_ref;
    http_parser_t*  http_parser_ptr;
    int             read_state;
    int             write_state;
    DC_Close_CB     on_close_cb;
    bool            cleanup_done_flag;
    bool            readside_posted;
    bool            writeside_posted;
    bool            post_active;
    int             read_buffer_size;

} AsyncConnection, *AsyncConnectionRef;

AsyncConnectionRef async_connection_new(
        int                 socket,
        ReactorRef          reactor_ref,
        AsyncHandlerRef     handler_ref
);
void async_connection_init(
        AsyncConnectionRef this,
        int                 socket,
        ReactorRef          reactor_ref,
        AsyncHandlerRef      handler_ref
);

void async_connection_free(AsyncConnectionRef this);
void async_connection_amonymous_dispose(void* p);
void async_connection_read(AsyncConnectionRef connection_ref); //, void(*on_read_message_cb)(void* href, MessageRef, int status));
void async_connection_write(AsyncConnectionRef connection_ref, MessageRef); //, void(*on_write_message_cb)(void* href, int status));


/**=============================================================================
 * AsyncHandler  api
 * Within an AsyncServer there is one instance of AsyncHandler for each
 * active connectio. These are created when an accept returns and
 * destroyed when a connection closes.
=============================================================================*/
typedef struct AsyncHandler_s {
    DECLARE_TAG;

    void(*handle_request)(AsyncHandlerRef, MessageRef);
    void(*handle_write_done)(AsyncHandlerRef);
    void(*handle_connection_done)(AsyncHandlerRef);

    void(*handle_reader_stopped)(AsyncHandlerRef);
    void(*handle_io_error)(AsyncHandlerRef);
    void(*handle_write_failed)(AsyncHandlerRef);

    AsyncConnectionRef  async_connection_ref;
    AsyncServerRef      server_ref;
    ListRef             input_list; // List of MessageRef - requests
    ListRef             output_list; // List of MessageRef - responses
    MessageRef          active_response;

} AsyncHandler, *AsyncHandlerRef;

AsyncHandlerRef async_handler_new(
        int socket,
        ReactorRef reactor_ref,
        AsyncServerRef sref
);
void async_handler_init(
        AsyncHandlerRef this, int socket,
        ReactorRef reactor_ref,
        AsyncServerRef sref
);
void async_handler_free(AsyncHandlerRef this);
void async_handler_anonymous_dispose(void** p);
inline ReactorRef async_handler_reactor_ref(AsyncHandlerRef handler_ref)
{
    return handler_ref->async_connection_ref->reactor_ref;
}
inline int async_handler_socket(AsyncHandlerRef handler_ref)
{
    return handler_ref->async_connection_ref->socket;
}
int async_handler_threadid(AsyncHandlerRef handler_ref);
#endif