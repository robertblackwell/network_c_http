

#include <rbl/macros.h>
#include <rbl/check_tag.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/demo_protocol/demo_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_process_request.h>

//static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request);
static void handle_request( void* href, DemoMessageRef msgref, int error_code);
static void postable_write_start(RunloopRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(RunloopRef reactor_ref, void* href);

static void connection_completion_cb(void* href)
{
    printf("file demo_handler.c connection_completion_cb\n");
    DemoHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
DemoHandlerRef demohandler_new(
        RunloopRef reactor_ref,
        int socket,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    DemoHandlerRef this = malloc(sizeof(DemoHandler));
    demohandler_init(this, reactor_ref, socket, completion_cb, server_ref);
    return this;
}
void demohandler_init(
        DemoHandlerRef this,
        RunloopRef runloop_ref,
        int socket,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    RBL_SET_TAG(DemoHandler_TAG, this)
    RBL_SET_END_TAG(DemoHandler_TAG, this)
    RBL_CHECK_TAG(DemoHandler_TAG, this)
    RBL_CHECK_END_TAG(DemoHandler_TAG, this)
    this->raw_socket = socket;
    this->demo_connection_ref = democonnection_new(
            runloop_ref,
            socket,
            connection_completion_cb,
            this
            );
    this->runloop_ref = runloop_ref;
    this->completion_callback = completion_cb;
    this->server_ref = server_ref;
    this->input_list = List_new(NULL);
    this->output_list = List_new(NULL);
    this->active_response = NULL;

    democonnection_read(this->demo_connection_ref, &handle_request, this);
}
void demohandler_free(DemoHandlerRef this)
{
    RBL_CHECK_TAG(DemoHandler_TAG, this)
    RBL_CHECK_END_TAG(DemoHandler_TAG, this)
    democonnection_free(this->demo_connection_ref);
    this->demo_connection_ref = NULL;
    List_dispose(&(this->input_list));
    List_dispose(&(this->output_list));
    free(this);
}
void demohandler_anonymous_dispose(void** p)
{
    DemoHandlerRef ref = *p;
    demohandler_free(ref);
    *p = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main driver functon - keeps everything going
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handle_request(void* href, DemoMessageRef msgref, int error_code)
{
    RBL_LOG_FMT("handler handle_request \n");
    DemoHandlerRef handler_ref = href;
    DemoMessageRef response = NULL;
    if(error_code) {
        printf("DemoHandler handler_request error_code %d\n", error_code);
    } else {
        response = process_request(handler_ref, msgref);
        demo_message_free(msgref);
        List_add_back(handler_ref->output_list, response);
        if(List_size(handler_ref->output_list) > 1) {
            assert(false);
        } else if (List_size(handler_ref->output_list) == 1) {
            runloop_post(handler_ref->runloop_ref, postable_write_start, href);
        } // why not else
    }
}
#if 0
static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request)
{
    RBL_CHECK_TAG(DemoHandler_TAG, href)
    RBL_CHECK_END_TAG(DemoHandler_TAG, href)
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
static void postable_write_start(RunloopRef runloop_ref, void* href)
{
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    pid_t tid = gettid();
    printf("postable_write_start tid: %d active_response:%p fd:%d  write_state: %d\n", tid, handler_ref->active_response, handler_ref->demo_connection_ref->asio_stream_ref->fd, handler_ref->demo_connection_ref->write_state);
    if(handler_ref->active_response != NULL) {
        return;
    }
    DemoMessageRef response = List_remove_first(handler_ref->output_list);
    RBL_ASSERT((handler_ref->active_response == NULL), "handler active response should be NULL");
    handler_ref->active_response = response;
    if(response != NULL) {
        democonnection_write(handler_ref->demo_connection_ref, response, on_write_complete_cb, href);
    }
}
static void on_write_complete_cb(void* href, int status)
{
    DemoHandlerRef handler_ref = href;
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    RBL_CHECK_END_TAG(DemoHandler_TAG, handler_ref)
    printf("on_write_complete_cb fd:%d  write_state:%d\n", handler_ref->demo_connection_ref->asio_stream_ref->fd, handler_ref->demo_connection_ref->write_state);
    demo_message_dispose(&(handler_ref->active_response));
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
    democonnection_read(handler_ref->demo_connection_ref, &handle_request, href);
}
