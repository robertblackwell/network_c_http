#include <apps/sync_msg_stream/sync_msg_stream.h>
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

GenericMsgRef make_request(VerifyThreadContextRef ctx, int i, int j);
void process_request(void* handler, GenericMsgRef request, GenericMsgRef reply);
bool verify_response(GenericMsgRef request, GenericMsgRef response);

void* verify_client_thread_function(void* data)
{
    VerifyThreadContext* ctx = (VerifyThreadContext*)data;
    rta_start_measurement(ctx->response_times_ref);
//    GenericMsgParserRef parser = generic_msg_parser_new();
    for(int i = 0; i < ctx->max_connections_per_thread; i++) {
        SyncMsgStreamRef sync_stream = sync_msg_stream_new();
        sync_msg_stream_connect(sync_stream, ctx->port);
        verify_ctx_reset_connection_round_trip_count(ctx);
        // verify_app_run(stream, ctx);
        int j = 0;
        while(1) {
            j++;
            rta_start_round_trip(ctx->response_times_ref);
            bool last_round = ctx->roundtrip_per_connection_counter + 1 == ctx->max_rountrips_per_connection;

            GenericMsgRef request = make_request(ctx, i, j);
            GenericMsgRef response = NULL;

            sync_msg_stream_send(sync_stream, request);
            response = sync_msg_stream_recv(sync_stream);

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
            generic_msg_free(request);
            request = NULL;
            if(response != NULL) {
                generic_msg_free(response);
                response = NULL;
            }
        }
        sync_msg_stream_free(sync_stream);
    }
    rta_end_measurement(ctx->response_times_ref);
    return NULL;
}
GenericMsgRef make_request(VerifyThreadContextRef ctx, int i, int j)
{
#if defined(MSG_SELECT_NEWLINE) || defined(MSG_SELECT_STX)
    char buf[100];
    char* buf_ptr = buf;
    make_uuid(&buf_ptr);
    sprintf(buf, "%s", buf_ptr);
    GenericMsgRef msg_ref = generic_msg_new();
    IOBufferRef iob = generic_msg_get_content(msg_ref);
    IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->ident, i, j);
    return msg_ref;
#elif defined(MSG_SELECT_HTTP)
    char* url = "http://somewhere.com/subfolder?a=1";
    return http_make_request(url, false);
#endif
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
#elif defined(MSG_SELECT_HTTP)
    HttpMessageRef response = http_message_new();

#endif
}
bool verify_response(GenericMsgRef request_msg, GenericMsgRef response_msg)
{
#if defined(MSG_SELECT_NEWELINE) || defined(MSG_SELECT_STX)
    IOBufferRef request_content = generic_msg_get_content(request_msg);
    IOBufferRef response_content = generic_msg_get_content(response_msg);
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
#elif defined(MSG_SELECT_HTTP)
    return http_verify_response(request_msg, response_msg);

#endif
}
