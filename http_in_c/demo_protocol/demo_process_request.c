

#include <http_in_c/demo_protocol/demo_handler.h>
#include <http_in_c/demo_protocol/demo_process_request.h>

DemoMessageRef demo_process_request(DemoHandlerRef href, DemoMessageRef request)
{
    RBL_CHECK_TAG(DemoHandler_TAG, href)
    RBL_CHECK_END_TAG(DemoHandler_TAG, href)
    DemoMessageRef reply = demo_message_new();
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = demo_message_get_body(request);
    IOBufferRef  iob = BufferChain_compact(request_body);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
//    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
    return reply;
}
