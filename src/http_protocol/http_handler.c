#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <runloop/runloop.h>
// #include <epoll_runloop/rl_internal.h>
#include <http_protocol/http_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <http_protocol/http_message.h>

static void on_new_message_handler( void* href, HttpMessageRef request, int error_code);
static void postable_write_start(RunloopRef runloop_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef runloop_ref, void* href);

static void connection_completion_cb(void* href)
{
    RBL_LOG_FMT("file http_handler.c connection_completion_cb\n");
    HttpHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
HttpHandlerRef http_handler_new(
        RunloopRef runloop_ref,
        int socket,
        HttpProcessRequestFunction handle_request,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* server_ref)
{
    HttpHandlerRef this = malloc(sizeof(HttpHandler));
    http_handler_init(this, runloop_ref, socket, handle_request, completion_cb, server_ref);
    return this;
}
void http_handler_init(
        HttpHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        HttpProcessRequestFunction request_handler,
        void(*completion_cb)(void*, HttpHandlerRef),
        void* server_ref)
{
    RBL_SET_TAG(HttpHandler_TAG, this)
    RBL_SET_END_TAG(HttpHandler_TAG, this)
    RBL_CHECK_TAG(HttpHandler_TAG, this)
    RBL_CHECK_END_TAG(HttpHandler_TAG, this)
    this->raw_socket = socket;
    this->http_connection_ref = http_connection_new(
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
    this->request_handler = request_handler;
    assert(this->request_handler != NULL);

    http_connection_read(this->http_connection_ref, on_new_message_handler, this);
}

void http_handler_free(HttpHandlerRef this)
{
    RBL_CHECK_TAG(HttpHandler_TAG, this)
    RBL_CHECK_END_TAG(HttpHandler_TAG, this)
    http_connection_free(this->http_connection_ref);
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
static void on_new_message_handler(void* href, HttpMessageRef request, int error_code)
{
    RBL_LOG_FMT("handler handle_request \n");
    HttpHandlerRef handler_ref = href;
    RBL_CHECK_TAG(HttpHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(HttpHandler_TAG, handler_ref)
    HttpConnectionRef cref = handler_ref->http_connection_ref;
    RBL_CHECK_TAG(HttpConnection_TAG, cref)
    RBL_CHECK_END_TAG(HttpConnection_TAG, cref)

    if(error_code) {
        RBL_LOG_FMT("HttpHandler handler_request error_code %d\n", error_code);
        handler_ref->completion_callback(handler_ref->server_ref, handler_ref);
    } else {
        HttpMessageRef response = http_message_new();
        assert(handler_ref->request_handler != NULL);
        handler_ref->request_handler(href, request, response);
        http_message_free(request);
        List_add_back(handler_ref->output_list, response);
        if(List_size(handler_ref->output_list) > 1) {
            assert(false);
        } else if (List_size(handler_ref->output_list) == 1) {
            runloop_post(handler_ref->runloop_ref, postable_write_start, href);
        } // why not else
    }
}
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
        http_connection_write(handler_ref->http_connection_ref, response, on_write_complete_cb, href);
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
    http_connection_read(handler_ref->http_connection_ref, &on_new_message_handler, href);
}
