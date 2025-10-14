#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <src/runloop/runloop.h>
// #include <src/runloop/rl_internal.h>
#include <src/demo_protocol/demo_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <src/demo_protocol/message.h>

static void on_new_message_handler( void* href, DemoMessageRef msgref, int error_code);
static void postable_write_start(RunloopRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef reactor_ref, void* href);

static void connection_completion_cb(void* href)
{
    RBL_LOG_FMT("file demo_handler.c connection_completion_cb\n");
    DemoHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
DemoHandlerRef demo_handler_new(
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    DemoHandlerRef this = malloc(sizeof(DemoHandler));
    demo_handler_init(this, runloop_ref, socket, request_handler, completion_cb, server_ref);
    return this;
}
void demo_handler_init(
        DemoHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        DemoProcessRequestFunction request_handler,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    RBL_SET_TAG(DemoHandler_TAG, this)
    RBL_SET_END_TAG(DemoHandler_TAG, this)
    RBL_CHECK_TAG(DemoHandler_TAG, this)
    RBL_CHECK_END_TAG(DemoHandler_TAG, this)
    this->raw_socket = socket;
    this->demo_connection_ref = demo_connection_new(
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

    demo_connection_read(this->demo_connection_ref, &on_new_message_handler, this);
}

void demo_handler_free(DemoHandlerRef this)
{
    RBL_CHECK_TAG(DemoHandler_TAG, this)
    RBL_CHECK_END_TAG(DemoHandler_TAG, this)
    demo_connection_free(this->demo_connection_ref);
    this->demo_connection_ref = NULL;
    // dont own this->server_ref
    // dont own runloop_ref
    assert(List_size(this->input_list) == 0);
    assert(List_size(this->output_list) == 0);
    List_free(this->input_list);
    List_free(this->output_list);
    free(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main driver function - keeps everything going
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void on_new_message_handler(void* href, DemoMessageRef request_ref, int error_code)
{
    RBL_LOG_FMT("handler handle_request \n");
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    DemoConnectionRef cref = handler_ref->demo_connection_ref;
    RBL_CHECK_TAG(DemoConnection_TAG, cref)
    RBL_CHECK_END_TAG(DemoConnection_TAG, cref)

    if(error_code) {
        RBL_LOG_FMT("DemoHandler handler_request error_code %d\n", error_code);
        handler_ref->completion_callback(handler_ref->server_ref, handler_ref);
    } else {
        DemoMessageRef response_ref = demo_message_new();
        assert(handler_ref->request_handler != NULL);
        handler_ref->request_handler(handler_ref, request_ref, response_ref);
        demo_message_free(request_ref);
        List_add_back(handler_ref->output_list, response_ref);
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
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    #ifdef __APPLE__
        pthread_t tid = pthread_self();
    #elif defined(__linux__)
        pid_t tid = gettid();
    #else
    #endif
    RBL_LOG_FMT("postable_write_start tid: %p active_response:%p fd:%d  write_state: %d\n", tid, handler_ref->active_response, 
        asio_stream_get_fd(handler_ref->demo_connection_ref->asio_stream_ref), handler_ref->demo_connection_ref->write_state);
    if(handler_ref->active_response != NULL) {
        return;
    }
    DemoMessageRef response = List_remove_first(handler_ref->output_list);
    RBL_ASSERT((handler_ref->active_response == NULL), "handler active response should be NULL");
    handler_ref->active_response = response;
    if(response != NULL) {
        demo_connection_write(handler_ref->demo_connection_ref, response, on_write_complete_cb, href);
    }
}
static void on_write_complete_cb(void* href, int status)
{
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    RBL_LOG_FMT("on_write_complete_cb fd:%d  write_state:%d\n", 
        asio_stream_get_fd(handler_ref->demo_connection_ref->asio_stream_ref), handler_ref->demo_connection_ref->write_state);
    demo_message_free(handler_ref->active_response);
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
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    demo_connection_read(handler_ref->demo_connection_ref, &on_new_message_handler, href);
}
