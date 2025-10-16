#include <client/verify_statistics.h>
#include <common/make_uuid.h>
#include <common/buffer_chain.h>
#include <client/verify_thread_context.h>
#include <sync_msg_stream/sync_msg_stream.h>
#include <msg/msg_selection_header.h>
#include <uuid/uuid.h>
#include "msg/demo_msg.h"

MSG_REF make_request();
void process_request(void* handler, MSG_REF request, MSG_REF reply);
bool verify_response(MSG_REF request, MSG_REF response);

void verify_app_run(SyncMsgStreamRef stream, VerifyThreadContext* ctx)
{
    while(1) {
        struct timeval iter_start_time = get_time();
        bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;
        MSG_REF request = make_request();
        MSG_REF response = NULL;
        int rc1 = sync_msg_stream_write(stream, request);
        if (rc1 != 0) break;
        int rc2 = sync_msg_stream_read(stream, &response);
        if (rc2 != 0) break;
        IOBufferRef iob_req = MSG_SERIALIZE(request);
        IOBufferRef iob_resp = MSG_SERIALIZE(response);
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
        MSG_FREE(request);
        request = NULL;
        if(response != NULL) {
            MSG_FREE(response);
            response = NULL;
        }
    }
}
MSG_REF make_request()
{
    MSG_REF request = MSG_NEW;
    uuid_t uuid;
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, buf_ptr);
    sprintf(buf, "%s", buf_ptr);
#if defined(MSG_SELECT_DEMO)
    demo_msg_set_is_request(request, true);
    BufferChainRef body = BufferChain_new();
#else

#endif
    IOBufferRef iob = IOBuffer_new();
    snprintf(IOBuffer_data(iob), IOBuffer_data_len(iob), "Hello from client: %s", buf_ptr);
    MSG_SET_CONTENT(request, iob);
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
void process_request(void* handler, MSG_REF request, MSG_REF reply)
{
#if defined(MSG_SELECT_DEMO)
    demo_msg_set_is_request(reply, false);
    BufferChainRef request_body = demo_msg_get_body(request);
    IOBufferRef  iob = BufferChain_compact(request_body);
    char opcode = *(char*)(IOBuffer_data(iob));
    *(char*)(IOBuffer_data(iob)) = 'R';
    BufferChainRef bc =  BufferChain_new();
    BufferChain_append_IOBuffer(bc, iob);
    IOBuffer_free(iob);
    //    BufferChain_append_bufferchain(bc, request_body);
    demo_msg_set_body(reply, bc);
#endif
}
bool verify_response(MSG_REF request, MSG_REF response)
{
#if 0
    BufferChainRef body = message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
#endif
    return true;
}
