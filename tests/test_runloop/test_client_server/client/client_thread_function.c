
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <rbl/logger.h>
#include <common/socket_functions.h>
#include <server/server_ctx.h>
#include <msg/msg_selection_header.h>
#include <mstream/mstream.h>
#include "../test_ctx.h"
#define MSTREAMX
_Thread_local MSG_REF new_msg = NULL;
void new_message_callback(void* arg, MSG_REF msg, int error_code)
{
    new_msg = msg;
}
MSG_REF make_request(TestCtxRef ctx, int i, int j);
void msg_write(int lfd, MSG_REF msg_ref);
MSG_REF msg_read(int fd, MSG_PARSER_REF parser);
int local_connect(const char* host, int port);
bool verify(MSG_REF request, MSG_REF response);

void* client_thread_function(void* tctx) {

    TestCtxRef ctx = tctx;
    int port = ctx->port;
    RBL_LOG_FMT("Client port: %d", ctx->port)
    for(int i = 0; i < ctx->nbr_connections; i++) {
#if defined(MSTREAM)
        MStreamRef mstream_ref = mstream_new("127.0.0.1", port);
#else
        int lfd = local_connect("127.0.0.1", port);
#endif
        MSG_PARSER_REF parser = MSG_PARSER_NEW();
        for (int j = 0; j < ctx->nbr_msg_per_connection; j++) {
            MSG_REF request_msg = make_request(ctx, i, j);
#if defined(MSTREAM)
            mstream_write(mstream_ref, request_msg);
#else
            msg_write(lfd, request_msg);
#endif
#if defined(MSTREAM)
            NewLineMsgRef response_msg = mstream_read(mstream_ref, parser);
#else
            MSG_REF response_msg = msg_read(lfd, parser);
#endif
            bool ok = verify(request_msg, response_msg);
            if(!ok) {
                ctx->error_count++;
            }
            MSG_FREE(request_msg);
            MSG_FREE(response_msg);
            printf("Test response %d id: %d i:%d j:%d\n", (int)((bool)ok), ctx->id, i, j);
        }
#if defined(MSTREAM)
        mstream_free(mstream_ref);
#else
        close(lfd);
#endif
    }
    return NULL;
}
MSG_REF make_request(TestCtxRef ctx, int i, int j)
{
    MSG_REF msg_ref = MSG_NEW;
    IOBufferRef iob = MSG_GET_CONTENT(msg_ref);
    IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->id, i, j);
    return msg_ref;
}
bool verify(MSG_REF request_msg, MSG_REF response_msg)
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
void msg_write(int lfd, MSG_REF msg_ref)
{
    IOBufferRef send_iob = MSG_SERIALIZE(msg_ref);
    size_t nw = write(lfd, IOBuffer_data(send_iob), IOBuffer_data_len(send_iob));
    assert(nw > 0);
}
MSG_REF msg_read(int lfd, MSG_PARSER_REF parser)
{
    IOBufferRef iob_response = IOBuffer_new(256);
    size_t rn = read(lfd, IOBuffer_space(iob_response), IOBuffer_space_len(iob_response));
    assert(rn > 0);
    IOBuffer_commit(iob_response, rn);
    MSG_PARSER_CONSUME(parser, iob_response, new_message_callback, NULL);
    assert(new_msg != NULL);
    IOBuffer_free(iob_response);
    iob_response = NULL;
    MSG_REF response_msg = new_msg;
    new_msg = NULL;
    return response_msg;
}
int local_connect(const char* host, int port)
{
    struct sockaddr_in server;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);
    const int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
    assert(status == 0);
    return lfd;
}