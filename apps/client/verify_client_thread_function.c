#include <mstream/mstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <common/make_uuid.h>
#include <rbl/logger.h>
#include "verify_statistics.h"
#include "verify_getopt.h"
#include "verify_thread_context.h"

MSG_REF make_request(VerifyThreadContextRef ctx, int i, int j);
void process_request(void* handler, MSG_REF request, MSG_REF reply);
bool verify_response(MSG_REF request, MSG_REF response);

void* verify_client_thread_function(void* data)
{
    VerifyThreadContext* ctx = (VerifyThreadContext*)data;
    rta_start_measurement(ctx->response_times_ref);
    MSG_PARSER_REF parser = MSG_PARSER_NEW();
    for(int i = 0; i < ctx->max_connections_per_thread; i++) {
        MStreamRef stream = mstream_new("127.0.0.1", ctx->port);
        verify_ctx_reset_connection_round_trip_count(ctx);
        // verify_app_run(stream, ctx);
        int j = 0;
        while(1) {
            j++;
            rta_start_round_trip(ctx->response_times_ref);
            bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;
            MSG_REF request = make_request(ctx, i, j);
            MSG_REF response = NULL;
            mstream_write(stream, request);
            // int rc1 = mstream_write(stream, request);
            // if (rc1 != 0) break;
            response = mstream_read(stream, parser);
            // int rc2 = mstream_read(stream, &response);
            // if (rc2 != 0) break;
            IOBufferRef iob_req = MSG_SERIALIZE(request);
            IOBufferRef iob_resp = MSG_SERIALIZE(response);
            if (verify_response(request, response)) {
                printf("Demo verify succeeded\n");
            } else {
                printf("Demo Verify response failed\n");
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
        mstream_free(stream);
    }
    rta_end_measurement(ctx->response_times_ref);
    return NULL;
}
MSG_REF make_request(VerifyThreadContextRef ctx, int i, int j)
{
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    sprintf(buf, "%s", buf_ptr);
    MSG_REF msg_ref = MSG_NEW;
    IOBufferRef iob = MSG_GET_CONTENT(msg_ref);
    IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->ident, i, j);
    return msg_ref;
}
void process_request(void* handler, MSG_REF request, MSG_REF reply)
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
bool verify_response(MSG_REF request_msg, MSG_REF response_msg)
{
    IOBufferRef request_content = MSG_GET_CONTENT(request_msg);
    IOBufferRef response_content = MSG_GET_CONTENT(response_msg);
    printf("[server] %s", IOBuffer_cstr(response_content));
    char test_buffer[200];
    int slen = sprintf(test_buffer, "ServerResponse:[%s]", IOBuffer_cstr(request_content));
    assert(slen == strlen(test_buffer));
    RBL_LOG_FMT("response:%s  send: %s ", IOBuffer_cstr(response_content), IOBuffer_cstr(request_content));
    bool ok = (bool)(0 == strcmp(test_buffer, IOBuffer_cstr(response_content)));
    if (!ok) {
        printf("Not ok\n");
    }
    return ok;
}
