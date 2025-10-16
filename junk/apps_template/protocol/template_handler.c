#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <src/runloop/runloop.h>
#include <src/runloop/rl_internal.h>
#include <src/tmpl_protocol/tmpl_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <src/tmpl_protocol/tmpl_message.h>

static void handle_request( void* href, TmplMessageRef msgref, int error_code);
static void postable_write_start(RunloopRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef reactor_ref, void* href);

static void connection_completion_cb(void* href)
{
    RBL_LOG_FMT("file tmpl_handler.c connection_completion_cb\n");
    TmplHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
TmplHandlerRef tmpl_handler_new(
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, TmplHandlerRef),
        void* server_ref)
{
    TmplHandlerRef this = malloc(sizeof(TmplHandler));
    tmpl_handler_init(this, runloop_ref, socket, completion_cb, server_ref);
    return this;
}
void tmpl_handler_init(
        TmplHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, TmplHandlerRef),
        void* server_ref)
{
    RBL_SET_TAG(TmplHandler_TAG, this)
    RBL_SET_END_TAG(TmplHandler_TAG, this)
    RBL_CHECK_TAG(TmplHandler_TAG, this)
    RBL_CHECK_END_TAG(TmplHandler_TAG, this)
    this->raw_socket = socket;
    this->tmpl_connection_ref = tmpl_connection_new(
            runloop_ref,
            socket,
            connection_completion_cb,
            this
            );
    this->request_handler = request_handler;
    this->runloop_ref = runloop_ref;
    this->completion_callback = completion_cb;
    this->server_ref = server_ref;
    this->input_list = List_new();
    this->output_list = List_new();
    this->active_response = NULL;
    assert(this->request_handler != NULL);

    tmpl_connection_read(this->tmpl_connection_ref, &handle_request, this);
}

void tmpl_handler_free(TmplHandlerRef this)
{
    RBL_CHECK_TAG(TmplHandler_TAG, this)
    RBL_CHECK_END_TAG(TmplHandler_TAG, this)
    tmpl_connection_free(this->tmpl_connection_ref);
    this->tmpl_connection_ref = NULL;
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
static void handle_request(void* href, TmplMessageRef request, int error_code)
{
    RBL_LOG_FMT("handler handle_request \n");
    TmplHandlerRef handler_ref = href;
    RBL_CHECK_TAG(TmplHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(TmplHandler_TAG, handler_ref)
    TmplConnectionRef cref = handler_ref->tmpl_connection_ref;
    RBL_CHECK_TAG(TmplConnection_TAG, cref)
    RBL_CHECK_END_TAG(TmplConnection_TAG, cref)

    if(error_code) {
        RBL_LOG_FMT("TmplHandler handler_request error_code %d\n", error_code);
        handler_ref->completion_callback(handler_ref->server_ref, handler_ref);
    } else {
        TmplMessageRef response_ref = tmpl_message_new();
        assert(handler_ref->request_handler != NULL);
        handler_ref->request_handler(handler_ref, request, resonse_ref);
        tmpl_message_free(request);
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
    TmplHandlerRef handler_ref = href;
    RBL_CHECK_TAG(TmplHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(TmplHandler_TAG, handler_ref)
    pid_t tid = gettid();
    RBL_LOG_FMT("postable_write_start tid: %d active_response:%p fd:%d  write_state: %d\n", tid, handler_ref->active_response, handler_ref->tmpl_connection_ref->asio_stream_ref->fd, handler_ref->tmpl_connection_ref->write_state);
    if(handler_ref->active_response != NULL) {
        return;
    }
    TmplMessageRef response = List_remove_first(handler_ref->output_list);
    RBL_ASSERT((handler_ref->active_response == NULL), "handler active response should be NULL");
    handler_ref->active_response = response;
    if(response != NULL) {
        tmpl_connection_write(handler_ref->tmpl_connection_ref, response, on_write_complete_cb, href);
    }
}
static void on_write_complete_cb(void* href, int status)
{
    TmplHandlerRef handler_ref = href;
    RBL_CHECK_TAG(TmplHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(TmplHandler_TAG, handler_ref)
    RBL_LOG_FMT("on_write_complete_cb fd:%d  write_state:%d\n", handler_ref->tmpl_connection_ref->asio_stream_ref->fd, handler_ref->tmpl_connection_ref->write_state);
    tmpl_message_free(handler_ref->active_response);
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
    TmplHandlerRef handler_ref = href;
    RBL_CHECK_TAG(TmplHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(TmplHandler_TAG, handler_ref)
    tmpl_connection_read(handler_ref->tmpl_connection_ref, &handle_request, href);
}
