

#include <http_in_c/http_protocol/http_handler.h>
#include <http_in_c/http_protocol/http_process_request.h>

HttpMessageRef process_request(HttpHandlerRef href, HttpMessageRef request)
{
    RBL_CHECK_TAG(HttpHandler_TAG, href)
    RBL_CHECK_END_TAG(HttpHandler_TAG, href)
    HttpMessageRef reply = HttpMessage_new();
    HttpMessage_set_is_request(reply, false);
    BufferChainRef request_body = HttpMessage_get_body(request);
    IOBufferRef  iob = BufferChain_compact(request_body);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
//    BufferChain_append_bufferchain(bc, request_body);
    HttpMessage_set_body(reply, bc);
    return reply;
}
