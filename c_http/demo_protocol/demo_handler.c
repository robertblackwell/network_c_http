
#define _GNU_SOURCE
#include <c_http/demo_protocol/demo_handler.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <c_http/macros.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/demo_protocol/demo_message.h>

static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request);
static void handle_request( void* href, DemoMessageRef msgref, int status);
static void postable_write_start(ReactorRef reactor_ref, void* href);
static void on_write_complete_cb(void* href, int status);
static void handler_postable_read_start(ReactorRef reactor_ref, void* href);

static void connection_completion_cb(void* href)
{
    printf("file demo_handler.c connection_completion_cb\n");
    DemoHandlerRef handler_ref = href;
    handler_ref->completion_callback(handler_ref->server_ref, href);
}
DemoHandlerRef demohandler_new(
        int socket,
        ReactorRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    DemoHandlerRef this = malloc(sizeof(DemoHandler));
    demohandler_init(this, socket, reactor_ref, completion_cb, server_ref);
    return this;
}
void demohandler_init(
        DemoHandlerRef this,
        int socket,
        ReactorRef reactor_ref,
        void(*completion_cb)(void*, DemoHandlerRef),
        void* server_ref)
{
    DEMO_HANDLER_SET_TAG(this)
    DEMO_HANDLER_CHECK_TAG(this)
    this->raw_socket = socket;
    this->demo_connection_ref = democonnection_new(
            socket,
            reactor_ref,
            this,
            connection_completion_cb);
    this->reactor_ref = reactor_ref;
    this->completion_callback = completion_cb;
    this->server_ref = server_ref;
    this->input_list = List_new(NULL);
    this->output_list = List_new(NULL);

    democonnection_read(this->demo_connection_ref, &handle_request);
}
void demohandler_free(DemoHandlerRef this)
{
    DEMO_HANDLER_CHECK_TAG(this)
    free(this);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main driver functon - keeps everything going
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handle_request(void* href, DemoMessageRef msgref, int error_code)
{
    LOG_FMT("handler handle_request \n");
    DemoHandlerRef handler_ref = href;
    DemoMessageRef response = NULL;
    if(error_code) {
        printf("DemoHandler handler_request error_code %d\n", error_code);
    } else {
        response = process_request(handler_ref, msgref);
        List_add_back(handler_ref->output_list, response);
        if (List_size(handler_ref->output_list) == 1) {
            rtor_reactor_post(handler_ref->reactor_ref, postable_write_start, href);
        }
        rtor_reactor_post(handler_ref->reactor_ref, handler_postable_read_start, href);
    }
}
static DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request)
{
    DEMO_HANDLER_CHECK_TAG(href)
    DemoMessageRef reply = demo_message_new();
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = demo_message_get_body(request);
    BufferChainRef bc = BufferChain_new();
    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void postable_write_start(ReactorRef reactor_ref, void* href)
{
    DemoHandlerRef handler_ref = href;
    DemoMessageRef response = List_remove_first(handler_ref->output_list);
    if(response != NULL) {
        democonnection_write(handler_ref->demo_connection_ref, response, on_write_complete_cb);
    }
}
static void on_write_complete_cb(void* href, int status)
{
    DemoHandlerRef handler_ref = href;
    if(List_size(handler_ref->output_list) == 1) {
        rtor_reactor_post(handler_ref->reactor_ref, postable_write_start, href);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read sequence
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void handler_postable_read_start(ReactorRef reactor_ref, void* href)
{
    DemoHandlerRef handler_ref = href;
    democonnection_read(handler_ref->demo_connection_ref, &handle_request);
}
