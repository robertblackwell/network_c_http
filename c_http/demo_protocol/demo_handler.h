#ifndef chttp_demo_handler_h
#define chttp_demo_handler_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/common/list.h>
#include <c_http/common/iobuffer.h>
#include <c_http/demo_protocol/demo_server.h>
#include <c_http/demo_protocol/demo_parser.h>

#define TYPE DemoHandler
#define DemoHandler_TAG "DmHDLR"
#include <c_http/check_tag.h>
#define DEMO_HANDLER_DECLARE_TAG DECLARE_TAG(DemoHandler_TAG)
#define DEMO_HANDLER_CHECK_TAG(p) CHECK_TAG(DemoHandler_TAG, p)
#define DEMO_HANDLER_SET_TAG(p) SET_TAG(DemoHandler_TAG, p)
#undef TYPE



typedef struct DemoHandler_s DemoHandler, *DemoHandlerRef;

typedef struct DemoHandler_s {
    DEMO_PARSER_DECLARE_TAG;
    int             raw_socket;
    ReactorRef      reactor_ref;
    RtorStreamRef   socket_stream_ref;
    DemoServerRef   server_ref;
    void(*completion_callback)(void* server_ref);
    ListRef         input_list;
    ListRef         output_list; // List of DemoMessageRef - responses
    IOBufferRef     active_input_buffer_ref;
    IOBufferRef     active_output_buffer_ref;
    DemoParserRef   parser_ref;
    bool            post_active;
    bool            write_eagained;
    bool            read_eagained;

} DemoHandler, *DemoHandlerRef;

DemoHandlerRef demohandler_new(int socket, ReactorRef reactor_ref, DemoServerRef server_ref);
void demohandler_init(DemoHandlerRef this, int socket, ReactorRef reactor_ref, DemoServerRef server_ref);
void demohandler_free(DemoHandlerRef this);
void demohandler_amonymous_dispose(void* p);

#endif