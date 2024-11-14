

#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/http_protocol/http_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/http_protocol/http_process_request.h>

//static HttpMessageRef process_request(HttpHandlerRef href, HttpMessageRef request);
static void handle_request( void* href, HttpMessageRef msgref, int error_code);
static void postable_write_start(RunloopRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef reactor_ref, void* href);

static void connection_completion_cb(void* href)
{
    RBL_LOG_FMT("file http_handler.c connection_completion_cb\n");
    HttpHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
HttpHandlerRef demohandler_new(
        RunloopRef reactor_ref,
        int socket,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* server_ref)
{
    HttpHandlerRef this = malloc(sizeof(HttpHandler));
    demohandler_init(this, reactor_ref, socket, completion_cb, server_ref);
    return this;
}
void demohandler_init(
        HttpHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* server_ref)
{
    RBL_SET_TAG(HttpHandler_TAG, this)
    RBL_SET_END_TAG(HttpHandler_TAG, this)
    RBL_CHECK_TAG(HttpHandler_TAG, this)
    RBL_CHECK_END_TAG(HttpHandler_TAG, this)
    this->raw_socket = socket;
    this->http_connection_ref = democonnection_new(
            runloop_ref,
            socket,
            connection_completion_cb,
            this
            );
    this->runloop_ref = runloop_ref;
    this->completion_callback = completion_cb;
    this->server_ref = server_ref;
    this->input_list = List_new();
    this->output_list = List_new();
    this->active_response = NULL;

    democonnection_read(this->http_connection_ref, &handle_request, this);
}

void demohandler_free(HttpHandlerRef this)
{
    RBL_CHECK_TAG(HttpHandler_TAG, this)
    RBL_CHECK_END_TAG(HttpHandler_TAG, this)
    democonnection_free(this->http_connection_ref);
    this->http_connection_ref = NULL;
    // dont own this->server_ref
    // dont own runloop_ref
    assert(List_size(this->input_list) == 0);
    assert(List_size(this->output_list) == 0);
    List_free(this->input_list);
    List_free(this->output_list);
    free(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main driver functon - keeps everything going
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handle_request(void* href, HttpMessageRef msgref, int error_code)
{
    RBL_LOG_FMT("handler handle_request \n");
    HttpHandlerRef handler_ref = href;
    HttpMessageRef response = NULL;
    if(error_code) {
        printf("HttpHandler handler_request error_code %d\n", error_code);
    } else {
        response = process_request(handler_ref, msgref);
        http_message_free(msgref);
        List_add_back(handler_ref->output_list, response);
        if(List_size(handler_ref->output_list) > 1) {
            assert(false);
        } else if (List_size(handler_ref->output_list) == 1) {
            runloop_post(handler_ref->runloop_ref, postable_write_start, href);
        } // why not else
    }
}
#if 0
static HttpMessageRef process_request(HttpHandlerRef href, HttpMessageRef request)
{
    RBL_CHECK_TAG(HttpHandler_TAG, href)
    RBL_CHECK_END_TAG(HttpHandler_TAG, href)
    HttpMessageRef reply = http_message_new();
    http_message_set_is_request(reply, false);
    BufferChainRef request_body = http_message_get_body(request);
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    http_message_set_body(reply, bc);
    return reply;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void postable_write_start(RunloopRef runloop_ref, void* href)
{
    HttpHandlerRef handler_ref = href;
    RBL_CHECK_TAG(HttpHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(HttpHandler_TAG, handler_ref)
    pid_t tid = gettid();
    RBL_LOG_FMT("postable_write_start tid: %d active_response:%p fd:%d  write_state: %d\n", tid, handler_ref->active_response, handler_ref->http_connection_ref->asio_stream_ref->fd, handler_ref->http_connection_ref->write_state);
    if(handler_ref->active_response != NULL) {
        return;
    }
    HttpMessageRef response = List_remove_first(handler_ref->output_list);
    RBL_ASSERT((handler_ref->active_response == NULL), "handler active response should be NULL");
    handler_ref->active_response = response;
    if(response != NULL) {
        democonnection_write(handler_ref->http_connection_ref, response, on_write_complete_cb, href);
    }
}
static void on_write_complete_cb(void* href, int status)
{
    HttpHandlerRef handler_ref = href;
    RBL_CHECK_TAG(HttpHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(HttpHandler_TAG, handler_ref)
    RBL_LOG_FMT("on_write_complete_cb fd:%d  write_state:%d\n", handler_ref->http_connection_ref->asio_stream_ref->fd, handler_ref->http_connection_ref->write_state);
    http_message_free(handler_ref->active_response);
    handler_ref->active_response = NULL;
    if(List_size(handler_ref->output_list) >= 1) {
        runloop_post(handler_ref->runloop_ref, postable_write_start, href);
    } else {
        runloop_post(handler_ref->runloop_ref, handler_postable_read_start, href);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handler_postable_read_start(RunloopRef runloop_ref, void* href)
{
    HttpHandlerRef handler_ref = href;
    RBL_CHECK_TAG(HttpHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(HttpHandler_TAG, handler_ref)
    democonnection_read(handler_ref->http_connection_ref, &handle_request, href);
}
