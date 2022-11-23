#ifndef chttp_demo_handler_h
#define chttp_demo_handler_h
#include <c_http/simple_runloop/runloop.h>
#include <c_http/common/list.h>
#include <c_http/common/iobuffer.h>
#include <c_http/demo_protocol/demo_server.h>
#include <c_http/demo_protocol/demo_parser.h>
typedef struct DemoHandler_s {
    int             raw_socket;
    ReactorRef      reactor_ref;
    RtorStreamRef   socket_stream_ref;
    DemoServerRef   server_ref;
    void(*completion_callback)(void* server_ref);
    ListRef         input_list; // List of DemoMessageRef
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