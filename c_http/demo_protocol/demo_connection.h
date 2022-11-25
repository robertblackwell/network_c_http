#ifndef chttp_demo_connection_h
#define chttp_demo_connection_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/common/list.h>
#include <c_http/common/iobuffer.h>
#include <c_http/demo_protocol/demo_server.h>
//#include <c_http/demo_protocol/demo_handler.h>
#include <c_http/demo_protocol/demo_parser.h>

#define TYPE DemoConnection
#define DemoConnection_TAG "DmCONN"
#include <c_http/check_tag.h>
#define DEMO_CONNECTION_DECLARE_TAG DECLARE_TAG(DemoConnection_TAG)
#define DEMO_CONNECTION_CHECK_TAG(p) CHECK_TAG(DemoConnection_TAG, p)
#define DEMO_CONNECTION_SET_TAG(p) SET_TAG(DemoConnection_TAG, p)
#undef TYPE



typedef struct DemoConnection_s DemoConnection, *DemoConnectionRef;

typedef struct DemoConnection_s {
    DEMO_PARSER_DECLARE_TAG;
    ReactorRef      reactor_ref;
    RtorStreamRef   socket_stream_ref;
    DemoHandlerRef  handler_ref;
    IOBufferRef     active_input_buffer_ref;
    IOBufferRef     active_output_buffer_ref;
    DemoParserRef   parser_ref;
    int             read_state;
    int             write_state;
    void(*on_read_cb)(void* href, DemoMessageRef, int status);
    void(*on_write_cb)(void* href, int status);
    void(*on_close_cb)(void* href);
    bool            readside_posted;
    bool            writeside_posted;
    bool            post_active;

} DemoConnection, *DemoConnectionRef;

DemoConnectionRef democonnection_new(
        int socket,
        ReactorRef reactor_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href));
void democonnection_init(
        DemoConnectionRef this,
        int socket,
        ReactorRef reactor_ref,
        DemoHandlerRef handler_ref,
        void(*connection_completion_cb)(void* href));
void democonnection_free(DemoConnectionRef this);
void democonnection_amonymous_dispose(void* p);

void democonnection_read(DemoConnectionRef connection_ref, void(*on_demo_read_cb)(void* href, DemoMessageRef, int statuc));
void democonnection_write(DemoConnectionRef connection_ref, DemoMessageRef, void(*on_demo_write_cb)(void* href, int statuc));

#endif