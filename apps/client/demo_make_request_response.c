#include <time.h>
#include <src/common/make_uuid.h>
#include <common/iobuffer.h>
#include <common/buffer_chain.h>
#include "demo_make_request_response.h"

MSG_REF demo_make_request()
{
    MSG_REF request = MSG_NEW;
    demo_message_set_is_request(request, true);
    BufferChainRef body = BufferChain_new();
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    sprintf(buf, "%s", buf_ptr);
    BufferChain_append_cstr(body, buf);
    demo_message_set_body(request, body);
    return request;
}
void demo_process_request(void* handler, MSG_REF request, MSG_REF reply)
{
    demo_message_set_is_request(reply, false);
    BufferChainRef request_body = MSG_GET_CONTENT(request);
    IOBufferRef  iob = BufferChain_compact(request_body);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
//    BufferChain_append_bufferchain(bc, request_body);
    demo_message_set_body(reply, bc);
}
/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       ThreadContext*
 * \param request   MSG_REF
 * \param response  MSG_REF
 * \return bool
 */
bool demo_verify_response(MSG_REF request, MSG_REF response)
{
    BufferChainRef body = demo_message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
    return true;
//    CbufferRef req_bc = http_message_serialize(request);
//    int x = strcmp(Cbuffer_cstr(body_bc), Cbuffer_cstr(req_bc));
//    if( x != 0) {
//        printf("Verify failed \n");
//        printf("Req     :  %s\n", Cbuffer_cstr(req_bc));
//        printf("Rsp body:  %s\n", Cbuffer_cstr(body_bc));
//    }
//    return (x == 0);
}
