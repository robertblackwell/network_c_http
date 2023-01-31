
#define _GNU_SOURCE
#include <c_http/demo_protocol/demo_handler.h>
#include <c_http/demo_protocol/demo_process_request.h>

DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request)
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
