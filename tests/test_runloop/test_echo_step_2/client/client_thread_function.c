
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <rbl/logger.h>
#include <common/socket_functions.h>
#include <server/server_ctx.h>
#include <msg/newline_msg.h>
#include "../test_ctx.h"
_Thread_local NewLineMsgRef new_msg = NULL;
void new_message_callback(void* arg, NewLineMsgRef msg, int error_code)
{
    new_msg = msg;
}
NewLineMsgRef make_request(TestCtxRef ctx, int i, int j);
void msg_write(int lfd, NewLineMsgRef newline_msg);
NewLineMsgRef msg_read(int fd, NewLineMsgParserRef parser);
bool verify(NewLineMsgRef request, NewLineMsgRef response);

void* client_thread_function(void* tctx) {

    TestCtxRef ctx = tctx;
    int port = ctx->port;
    RBL_LOG_FMT("Client port: %d", ctx->port)
    for(int i = 0; i < ctx->nbr_connections; i++) {
        struct sockaddr_in server;
        int lfd = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = port;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        const int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
        assert(status == 0);
        NewLineMsgParserRef parser = newline_msg_parser_new();
        for (int j = 0; j < ctx->nbr_msg_per_connection; j++) {
            NewLineMsgRef request_msg = make_request(ctx, i, j);
            // NewLineMsgRef newline_msg = newline_msg_new(256);
            // IOBufferRef iob = newline_msg_get_content(newline_msg);
            // IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->id, i, j);
            // IOBufferRef send_iob = newline_msg_serialize(newline_msg);
            // IOBufferRef saved_send_iob = IOBuffer_from_cstring((char*)IOBuffer_cstr(iob));

            msg_write(lfd, request_msg);
            // IOBufferRef send_iob = newline_msg_serialize(newline_msg);
            // size_t nw = write(lfd, IOBuffer_data(send_iob), IOBuffer_data_len(send_iob));
            // assert(nw > 0);

#if 1
            NewLineMsgRef response_msg = msg_read(lfd, parser);
#else
            IOBufferRef iob_response = IOBuffer_new(256);
            size_t rn = read(lfd, IOBuffer_space(iob_response), IOBuffer_space_len(iob_response));
            assert(rn > 0);
            IOBuffer_commit(iob_response, rn);
            newline_msg_parser_consume(parser, iob_response, new_message_callback, NULL);
            assert(new_msg != NULL);
            IOBuffer_free(iob_response);
            iob_response = NULL;
            NewLineMsgRef response_msg = new_msg;
            new_msg = NULL;
#endif
            bool ok = verify(request_msg, response_msg);
            // IOBufferRef response_content = newline_msg_get_content(response_msg);
            // printf("[server] %s", IOBuffer_cstr(response_content));
            // char test_buffer[200];
            // sprintf(test_buffer, "ServerResponse:[%s]", IOBuffer_cstr(saved_send_iob));
            // RBL_LOG_FMT("response:%s  send: %s ", IOBuffer_cstr(response_content), IOBuffer_cstr(send_iob));
            // int ok = strcmp(test_buffer, IOBuffer_cstr(response_content));

            if(!ok) {
                ctx->error_count++;
            }
            printf("Test response %d id: %d i:%d j:%d\n", (int)((bool)ok), ctx->id, i, j);
        }
        close(lfd);
    }
    return NULL;
}
NewLineMsgRef make_request(TestCtxRef ctx, int i, int j)
{
    NewLineMsgRef newline_msg = newline_msg_new(256);
    IOBufferRef iob = newline_msg_get_content(newline_msg);
    IOBuffer_sprintf(iob, "Client %d connection: %d msg: %d", ctx->id, i, j);
    return newline_msg;
}
bool verify(NewLineMsgRef request_msg, NewLineMsgRef response_msg)
{
    IOBufferRef request_content = newline_msg_get_content(request_msg);
    IOBufferRef response_content = newline_msg_get_content(response_msg);
    printf("[server] %s", IOBuffer_cstr(response_content));
    char test_buffer[200];
    sprintf(test_buffer, "ServerResponse:[%s]", IOBuffer_cstr(request_content));
    RBL_LOG_FMT("response:%s  send: %s ", IOBuffer_cstr(response_content), IOBuffer_cstr(request_content));
    bool ok = (bool)strcmp(test_buffer, IOBuffer_cstr(response_content));
    return ok;
}
void msg_write(int lfd, NewLineMsgRef newline_msg)
{
    IOBufferRef send_iob = newline_msg_serialize(newline_msg);
    size_t nw = write(lfd, IOBuffer_data(send_iob), IOBuffer_data_len(send_iob));
    assert(nw > 0);
}
NewLineMsgRef msg_read(int lfd, NewLineMsgParserRef parser)
{
    IOBufferRef iob_response = IOBuffer_new(256);
    size_t rn = read(lfd, IOBuffer_space(iob_response), IOBuffer_space_len(iob_response));
    assert(rn > 0);
    IOBuffer_commit(iob_response, rn);
    newline_msg_parser_consume(parser, iob_response, new_message_callback, NULL);
    assert(new_msg != NULL);
    IOBuffer_free(iob_response);
    iob_response = NULL;
    NewLineMsgRef response_msg = new_msg;
    new_msg = NULL;
    return response_msg;
}