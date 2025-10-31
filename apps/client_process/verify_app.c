#include "verify_statistics.h"
#include <common/make_uuid.h>
#include <common/buffer_chain.h>
#include "verify_thread_context.h"
#include <apps/tcp_msg_stream/msg_stream.h>
#include <uuid/uuid.h>

GenericMsgRef make_request();
void process_request(void* handler, GenericMsgRef request, GenericMsgRef reply);
bool verify_response(GenericMsgRef request, GenericMsgRef response);

void verify_app_run(MsgStreamRef stream, VerifyThreadContext* ctx)
{
    while(1) {
        struct timeval iter_start_time = get_time();
        bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;
        GenericMsgRef request = make_request();
        GenericMsgRef response = NULL;
        msg_stream_write(stream, request);
        // int rc1 = mstream_write(stream, request);
        // if (rc1 != 0) break;
        response = msg_stream_read(stream, parser);
        // int rc2 = mstream_read(stream, &response);
        // if (rc2 != 0) break;
        IOBufferRef iob_req = generic_msg_serialize(request);
        IOBufferRef iob_resp = generic_msg_serialize(response);
        if (verify_response(request, response)) {
            printf("Demo Verify response failed");
        } else {
            printf("Demo verify succeeded\n");
        }
        rta_end_round_trip(ctx->response_times_ref);
        verify_ctx_increment_connection_round_trip_count(ctx);
        verify_ctx_increment_total_round_trip_count(ctx);
        if(ctx->roundtrip_per_connection_counter >= ctx->max_rountrips_per_connection) {
            break;
        }
        generic_msg_free(request);
        request = NULL;
        if(response != NULL) {
            generic_msg_free(response);
            response = NULL;
        }
    }
}
GenericMsgRef make_request()
{
    uuid_t uuid;
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, buf_ptr);
    sprintf(buf, "%s", buf_ptr);
#if defined(MSG_SELECT_DEMO) || defined(MSG_SELECT_NEWLINE)
    GenericMsgRef msg_ref = generic_msg_new();
    IOBufferRef iob = MSG_GET_CONTENT(msg_ref);
    IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->id, i, j);
    return msg_ref;
#elif defined(MSG_SELECT_HTTP)
#endif
    IOBufferRef iob = IOBuffer_new();
    snprintf(IOBuffer_data(iob), IOBuffer_data_len(iob), "Hello from client: %s", buf_ptr);
    generic_msg_set_content(request, iob);
#if 0
    uuid_t uuid
    char buf[100];
    char* buf_ptr = buf;
    uuid_generate_random(uuid)
    uuid_parser_lower(uuid, buf_ptr);
    sprintf(buf, "%s", buf_ptr);
    BufferChain_append_cstr(body, buf);
    message_set_body(request, body);
#endif
    return request;
}
void process_request(void* handler, GenericMsgRef request, GenericMsgRef reply)
{
#if defined(MSG_SELECT_DEMO)
    demo_msg_set_is_request(reply, false);
    IOBufferRef  iob = demo_msg_get_body(request);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
    //    BufferChain_append_bufferchain(bc, request_body);
    demo_msg_set_body(reply, iob);
#endif
}
bool verify_response(GenericMsgRef request, GenericMsgRef response)
{
#if 0
    BufferChainRef body = message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
#endif
    return true;
}
