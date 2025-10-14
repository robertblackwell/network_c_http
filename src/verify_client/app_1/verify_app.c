#include <common/verify_statistics.h>
#include "../verify_thread_context.h"
#include "../sync_msg_stream.h"

MessageRef make_request();
void process_request(void* handler, MessageRef request, MessageRef reply);
bool verify_response(MessageRef request, MessageRef response);

void verify_app_run(SyncMsgStreamRef stream, VerifyThreadContext* ctx)
{
    while(1) {
        struct timeval iter_start_time = get_time();
        bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;
        MessageRef request = make_request();
        MessageRef response = NULL;
        int rc1 = sync_msg_stream_write(stream, request);
        if (rc1 != 0) break;
        int rc2 = sync_msg_stream_read(stream, &response);
        if (rc2 != 0) break;
        IOBufferRef iob_req = message_serialize(request);
        IOBufferRef iob_resp = message_serialize(response);
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
        message_free(request);
        request = NULL;
        if(response != NULL) {
            message_free(response);
            response = NULL;
        }
    }
}
MessageRef demo_make_request()
{
    MessageRef request = message_new();
    message_set_is_request(request, true);
    BufferChainRef body = BufferChain_new();
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    sprintf(buf, "%s", buf_ptr);
    BufferChain_append_cstr(body, buf);
    message_set_body(request, body);
    return request;
}
void demo_process_request(void* handler, MessageRef request, MessageRef reply)
{
    message_set_is_request(reply, false);
    BufferChainRef request_body = message_get_body(request);
    IOBufferRef  iob = BufferChain_compact(request_body);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
    //    BufferChain_append_bufferchain(bc, request_body);
    message_set_body(reply, bc);
}
bool demo_verify_response(MessageRef request, MessageRef response)
{
    BufferChainRef body = message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
    return true;
}
