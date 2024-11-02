

#define RBL_LOG_ENABLE
#include <http_in_c/async/async.h>
#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <rbl/logger.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>

//static AsyncMessageRef process_request(AsyncHandlerRef href, AsyncMessageRef request);
static void handle_request(AsyncHandlerRef href, MessageRef msgref);
void async_handler_handle_response(AsyncHandlerRef href, MessageRef request_ptr, MessageRef response_ptr);
static void handle_write_complete(AsyncHandlerRef href);

//static void handle_io_error(AsyncHandlerRef  href);
//static void handle_reader_stopped(AsyncHandlerRef  href);
static void handle_close_connection(AsyncHandlerRef  href);
//static void handle_write_failed(AsyncHandler* href);

static void postable_write_start(RunloopRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef reactor_ref, void* href);

AsyncHandlerRef async_handler_new(
        int socket,
        RunloopRef reactor_ref,
        AsyncServerRef server_ref)
{
    AsyncHandlerRef this = malloc(sizeof(AsyncHandler));
    async_handler_init(this, socket, reactor_ref, server_ref);
    return this;
}
void async_handler_init(
        AsyncHandlerRef this,
        int socket,
        RunloopRef reactor_ref,
        AsyncServerRef server_ref)
{
    RBL_SET_TAG(AsyncHandler_TAG, this)
    RBL_CHECK_TAG(AsyncHandler_TAG, this)
    // set up the delegate function
    this->handle_request = &handle_request;
    this->handle_response = &async_handler_handle_response;
    this->handle_write_complete = handle_write_complete;
    this->handle_close_connection = handle_close_connection;

    this->async_connection_ref = async_connection_new(
            socket,
            reactor_ref,
            this
);
    this->server_ref = server_ref;
    this->input_list = List_new(NULL);
    this->output_list = List_new(NULL);

    RBL_LOG_FMT("socket: %d", socket);
    async_connection_read(this->async_connection_ref);
}
void async_handler_destroy(AsyncHandlerRef this)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, this)
    async_connection_free(this->async_connection_ref);
    this->async_connection_ref = NULL;
    List_dispose(&(this->input_list));
    List_dispose(&(this->output_list));
    RBL_INVALIDATE_TAG(this)
    // RBL_INVALIDATE_STRUCT(this, AsyncHandler)
}
void async_handler_free(AsyncHandlerRef this)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, this)
    async_handler_destroy(this);
    free(this);
}
void async_handler_anonymous_dispose(void** p)
{
    AsyncHandlerRef ref = *p;
    async_handler_free(ref);
    *p = NULL;
}
/**
 * Called by async_connection when a new message arrives
 */
static void handle_request(AsyncHandlerRef href, MessageRef request_ptr)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOGFMT("href: %p socket:%d", href, href->async_connection_ref->socket);
    AsyncHandlerRef handler_ref = href;
    handler_ref->server_ref->process_request(handler_ref, request_ptr);
}
/**
 * Called by process_request() after it has constructed the response message
 * to have the response sent to the client
 */
void async_handler_handle_response(AsyncHandlerRef href, MessageRef request_ptr, MessageRef response_ptr)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, href)
    int cmp_tmp = Message_cmp_header(request_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    RBL_LOG_FMT("href: %p socket:%d keep alive: %d", href, href->async_connection_ref->socket, cmp_tmp);
    if(cmp_tmp == 1) {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    } else {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
    }

    async_connection_write(href->async_connection_ref, response_ptr);
}
/**
 * Called by async_connection when writing of the response is complete
 */
static void handle_write_complete(AsyncHandlerRef href)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOGFMT("href: %p socket:%d", href, href->async_connection_ref->socket);
    async_connection_read(href->async_connection_ref);
}
/**
 * Called by async_connection when the connection is ready to be closed and
 * no more activity will take place. Such as after an io error or peer closed connection
 *
 * Handler should call the server_ref->handler_complete() which will async_handler_dispose() the handler
 * which will async_connection_dipose() the connection which will close() the connection.
 *
 * After this chain of disposes no activity should take place with the connection, handler or runloop.
 *
 * NOTE: this function should be called directly from the runloop (ie posted) so that when it returns
 * no other async_connection or async_handler code is executed for this socket.
 */
static void handle_close_connection(AsyncHandlerRef href)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, href)
    RBL_ASSERT((href->async_connection_ref->socket_stream_ref > 0), "sockets fd must be still owned at this point");
    RBL_LOGFMT("file async_handler.c handle_close_connection socket %d", href->async_connection_ref->socket);
    href->server_ref->handler_complete(href->server_ref, href);
}
#if 0
// Deprecated
static void handle_io_error(AsyncHandlerRef  href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOG_FMT("handle_io_error");
}
// deprecated
static void handle_reader_stopped(AsyncHandlerRef  href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOG_FMT("handle_reader_stopped");
}
// deprecated
static void handle_write_failed(AsyncHandler* href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOG_FMT("handle_write_failed");
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// misc functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int async_handler_threadid(AsyncHandlerRef handler_ref)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, handler_ref)
    AsyncServerRef s = handler_ref->server_ref;
    return s->listening_socket_fd;
}
