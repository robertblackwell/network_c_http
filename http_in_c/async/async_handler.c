
#define _GNU_SOURCE
#define CHLOG_ON
#include <http_in_c/async/async.h>
#include <http_in_c/macros.h>
#include <http_in_c/check_tag.h>
#include <http_in_c/logger.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>

//static AsyncMessageRef process_request(AsyncHandlerRef href, AsyncMessageRef request);
static void handle_request(AsyncHandlerRef href, MessageRef msgref);
static void handle_write_done(AsyncHandlerRef href);
static void handle_connection_done(AsyncHandlerRef href);

static void handle_io_error(AsyncHandlerRef  href);
static void handle_reader_stopped(AsyncHandlerRef  href);
static void handle_write_failed(AsyncHandler* href);

static void postable_write_start(ReactorRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(ReactorRef reactor_ref, void* href);

AsyncHandlerRef async_handler_new(
        int socket,
        ReactorRef reactor_ref,
        AsyncServerRef server_ref)
{
    AsyncHandlerRef this = malloc(sizeof(AsyncHandler));
    async_handler_init(this, socket, reactor_ref, server_ref);
    return this;
}
void async_handler_init(
        AsyncHandlerRef this,
        int socket,
        ReactorRef reactor_ref,
        AsyncServerRef server_ref)
{
    SET_TAG(AsyncHandler_TAG, this)
    CHECK_TAG(AsyncHandler_TAG, this)
    // setup the delegate function
    this->handle_request = &handle_request;
    this->handle_write_done = handle_write_done;
    this->handle_connection_done = handle_connection_done;

    this->handle_reader_stopped = &handle_reader_stopped;
    this->handle_io_error = &handle_io_error;
    this->handle_write_failed = handle_write_failed;
    this->async_connection_ref = async_connection_new(
            socket,
            reactor_ref,
            this
);
    this->server_ref = server_ref;
    this->input_list = List_new(NULL);
    this->output_list = List_new(NULL);
    this->active_response = NULL;

    async_connection_read(this->async_connection_ref);//, &handle_request);
}
void async_handler_free(AsyncHandlerRef this)
{
    CHECK_TAG(AsyncHandler_TAG, this)
    async_connection_free(this->async_connection_ref);
    this->async_connection_ref = NULL;
    List_dispose(&(this->input_list));
    List_dispose(&(this->output_list));
    free(this);
}
void async_handler_anonymous_dispose(void** p)
{
    AsyncHandlerRef ref = *p;
    async_handler_free(ref);
    *p = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main driver functon - keeps everything going
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handle_request(AsyncHandlerRef href, MessageRef request_ptr)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    LOG_FMT("handler handle_request");
    AsyncHandlerRef handler_ref = href;
    MessageRef response_ptr = NULL;
    response_ptr = handler_ref->server_ref->process_request(handler_ref, request_ptr);
    int cmp_tmp = Message_cmp_header(request_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    if(cmp_tmp == 1) {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    } else {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
    }

    List_add_back(handler_ref->output_list, response_ptr);
    if (List_size(handler_ref->output_list) == 1) {
        rtor_reactor_post(handler_ref->async_connection_ref->reactor_ref, postable_write_start, href);
    }
    MessageRef m = request_ptr;
    Message_dispose(&m);
    rtor_reactor_post(handler_ref->async_connection_ref->reactor_ref, handler_postable_read_start, href);
}
#if 0
static AsyncMessageRef process_request(DemoHandlerRef href, DemoMessageRef request)
{
    CHECK_TAG(DemoHandler_TAG, href)
    DemoMessageRef reply = demo_message_new();
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = demo_message_get_body(request);
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
    return reply;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void postable_write_start(ReactorRef reactor_ref, void* href)
{
    AsyncHandlerRef handler_ref = href;
    LOG_FMT("postable_write_start active_response:%p fd:%d  write_state: %d", handler_ref->active_response, handler_ref->async_connection_ref->socket_stream_ref->fd, handler_ref->async_connection_ref->write_state);
    if(handler_ref->active_response != NULL) {
        LOG_FMT("active_response is not NULL")
        return;
    }
    MessageRef response = List_remove_first(handler_ref->output_list);
    CHTTP_ASSERT((handler_ref->active_response == NULL), "handler active response should be NULL");
    handler_ref->active_response = response;
    CHTTP_ASSERT((handler_ref->active_response != NULL), "handler active response should be NULL");
    if(response != NULL) {
        async_connection_write(handler_ref->async_connection_ref, response); //, on_write_complete_cb);
    }
}
static void handle_write_done(AsyncHandlerRef href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    AsyncHandlerRef handler_ref = href;
    LOG_FMT("handle_write_done fd:%d  write_state:%d  list size: %d",
            handler_ref->async_connection_ref->socket_stream_ref->fd,
            handler_ref->async_connection_ref->write_state,
            List_size(handler_ref->output_list)
            );
    Message_dispose(&(handler_ref->active_response));
    if(List_size(handler_ref->output_list) == 1) {
        rtor_reactor_post(handler_ref->async_connection_ref->reactor_ref, postable_write_start, href);
    }
}
static void handle_connection_done(AsyncHandlerRef href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    printf("file demo_handler.c connection_completion_cb\n");
    AsyncHandlerRef handler_ref = href;
    handler_ref->server_ref->handler_complete(handler_ref->server_ref, href);
}
static void handle_io_error(AsyncHandlerRef  href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    printf("handle_io_error\n");
    return;
    CHTTP_ASSERT(false, "not implemented");
}
static void handle_reader_stopped(AsyncHandlerRef  href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    printf("handle_reader_stopped\n");
    return;
    CHTTP_ASSERT(false, "not implemented");
}
static void handle_write_failed(AsyncHandler* href)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    printf("handle_write_failed\n");
    return;
    CHTTP_ASSERT(false, "not implemented");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handler_postable_read_start(ReactorRef reactor_ref, void* href)
{
    AsyncHandlerRef handler_ref = href;
    CHECK_TAG(AsyncHandler_TAG, handler_ref);
    async_connection_read(handler_ref->async_connection_ref);//, &handle_request);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// misc functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int async_handler_threadid(AsyncHandlerRef handler_ref)
{
    CHECK_TAG(AsyncHandler_TAG, handler_ref)
    AsyncServerRef s = handler_ref->server_ref;
    return s->listening_socket_fd;
}
