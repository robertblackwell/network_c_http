
#include <pthread.h>
#ifdef APPLE_FLAG
#include <sys/_pthread/_pthread_t.h>
#endif
#include <rbl/unittest.h>
#include "server/server_ctx.h"

typedef struct TestCtx {
    int     port;
    int     id;
    int     nbr_connections;
    int     nbr_msg_per_connection;
    int error_count;
} TestCtx, *TestCtxRef;

void testctx_init(TestCtxRef ctx, int id, int nbr_connections, int nbr_messages_per_connection) {
    ctx->port = 9002;
    ctx->id = id;
    ctx->nbr_connections = nbr_connections;
    ctx->nbr_msg_per_connection = nbr_messages_per_connection;
    ctx->error_count = 0;
}

void* client_thread_function(void* tctx);
void* server_thread_function(void* tctx);
int test_echo();

int main() {
    UT_ADD(test_echo);
    int rc = UT_RUN();
    return rc;
}
int test_echo() {
    TestCtx tctx[2];
    testctx_init(&tctx[0], 11, 4, 4);
    testctx_init(&tctx[1], 22, 4, 4);
    pthread_t server_thread;
    pthread_t client_1_thread;
    pthread_t client_2_thread;
    pthread_create(&server_thread, NULL, server_thread_function, &(tctx[0]));
    sleep(3);
    pthread_create(&client_1_thread, NULL, client_thread_function, &(tctx[0]));
    pthread_create(&client_2_thread, NULL, client_thread_function, &(tctx[1]));
    pthread_join(client_1_thread, NULL);
    pthread_join(client_2_thread, NULL);
    UT_EQUAL_INT(tctx[0].error_count, 0);
    UT_EQUAL_INT(tctx[1].error_count, 0);
    pthread_join(server_thread, NULL);
    printf("After all joins\n");
    return 0;
}


NewlineMsgParserRef g_parser_ref;
static NewlineMsgRef g_new_msg_ref = NULL;
typedef struct SyncMsgStream_s {
    int fd;
    NewlineMsgParserRef parser_ref;
    NewlineMsgRef new_msg;
} SyncMsgStream, *SyncMsgStreamRef;
void new_msg_callback(void* arg, NewlineMsgRef new_msg, int error)
{
    SyncMsgStreamRef sstream = arg;
    assert(sstream->new_msg == NULL);
    sstream->new_msg = new_msg;
}
SyncMsgStreamRef syncmsg_stream_new(int fd)
{
    SyncMsgStreamRef s = malloc(sizeof(SyncMsgStream));
    s->fd = fd;
    s->parser_ref = newline_msg_parser_new(new_msg_callback, s);
    return s;
}
void syncmsg_stream_free(SyncMsgStreamRef s)
{
    newline_msg_parser_free(s->parser_ref);
    free(s);
}
bool do_one_msg(SyncMsgStreamRef sync_stream, TestCtxRef ctx, int lfd, int ident, int i, int j);

void* client_thread_function(void* tctx) {

    TestCtxRef ctx = tctx;
    int port = ctx->port;
    int ident = ctx->id;

    for(int i = 0; i < ctx->nbr_connections; i++) {
        struct sockaddr_in server;
        int lfd = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = port;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        const int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
        assert(status == 0);
        SyncMsgStreamRef sync_stream = syncmsg_stream_new(lfd);
        for (int j = 0; j < ctx->nbr_msg_per_connection; j++) {
#if 1
            bool ok = do_one_msg(sync_stream, ctx, lfd, ctx->id, i, j);
            printf("Test response %d id: %d i:%d j:%d\n", (int)ok, ident, i, j);
#else
            #if 1
            char* send_msg_ptr = NULL;
            int sl = asprintf(&send_msg_ptr, "Client %d connection: %d msg: %d", ctx->id, i, j);
            #else
            send_msg[300] = "";
            snprintf(send_msg, 290, "Client %d connection: %d msg: %d", ctx->id, i, j);
            char send_buffer[300];
            snprintf(send_buffer, 290, "%s\n", send_msg);
            size_t lenx = strlen(send_buffer);
            #endif
            char* send_buff_ptr = NULL;
            size_t sl2 = asprintf(&send_buff_ptr, "%s\n", send_msg_ptr);

            printf("sending len:%lu msg:%s", sl2, send_buff_ptr);
            send(lfd, send_buff_ptr, sl2, 0);
            char receive_buffer[200];
            recv(lfd, receive_buffer, sizeof receive_buffer, 0);
            printf("[server] %s", receive_buffer);
            char test_buffer[200];
            sprintf(test_buffer, "ServerResponse:[%s]\n\n", send_msg_ptr);
            int ok = strcmp(test_buffer, receive_buffer);
             free(send_buff_ptr);
             free(send_msg_ptr);
            if(ok != 0) {
                ctx->error_count++;
            }
//            printf("Test response %d id: %d i:%d j:%d\n", ok, ident, i, j);
#endif
        }
        close(lfd);
    }
    printf("Client thread function exiting\n");
    return NULL;
}
void* server_thread_function(void* tctx)
{
    TestCtxRef ctx = (TestCtxRef)tctx;
    int port = ctx->port;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = server_ctx_new(runloop, fd);
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, 5000L);
    printf("XXXXXserver thread after runloop_run()\n");
    server_ctx_free(server_ctx_ref);
    runloop_free(runloop);
    return 0;
}
NewlineMsgRef make_send_msg(int ident, int i, int j)
{
    char* send_content_ptr = NULL;
    int sl = asprintf(&send_content_ptr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ Client %d connection: %d msg: %d", ident, i, j);
    assert(sl != -1);
    NewlineMsgRef msgref = newline_msg_new();
    IOBufferRef iob = IOBuffer_from_cstring(send_content_ptr);
    newline_msg_set_content(msgref, iob);
    return msgref;
}
void send_msg(SyncMsgStreamRef sync_stream, NewlineMsgRef msgref)
{
    IOBufferRef iob = newline_msg_serialize(msgref);
    ssize_t len = send(sync_stream->fd, IOBuffer_data(iob), IOBuffer_data_len(iob), 0);
    assert(len == IOBuffer_data_len(iob));
}
NewlineMsgRef recv_msg(SyncMsgStreamRef sync_stream)
{
    IOBufferRef iob = IOBuffer_new_with_capacity(1024);
    while(1) {
        IOBuffer_reset(iob);
        ssize_t len = recv(sync_stream->fd, IOBuffer_space(iob), IOBuffer_space_len(iob), 0);
        assert(len > 0);
        IOBuffer_commit(iob, (int)len);
        newline_msg_parser_consume(sync_stream->parser_ref, iob);
        assert(IOBuffer_data_len(iob) == 0);
        if (sync_stream->new_msg != NULL) {
            NewlineMsgRef tmp = sync_stream->new_msg;
            sync_stream->new_msg = NULL;
            IOBuffer_free(iob);
            return tmp;
        }
    }
}
bool verify(NewlineMsgRef sendmsg, NewlineMsgRef response)
{
    const char* send_cstr = IOBuffer_cstr(newline_msg_get_content(sendmsg));
    const char* response_cstr = IOBuffer_cstr(newline_msg_get_content(response));
    char* test_buffer_ptr;
    ssize_t len = asprintf(&test_buffer_ptr, "ServerResponse:[%s]", send_cstr);
    assert(len != -1);
    int l1 = strlen(test_buffer_ptr);
    int l2 = strlen(response_cstr);
    int x = strcmp(test_buffer_ptr, response_cstr);
    bool ok = (strcmp(test_buffer_ptr, response_cstr) ==0);
    free(test_buffer_ptr);
    return ok;
}
bool do_one_msg(SyncMsgStreamRef sync_stream, TestCtxRef ctx, int lfd, int ident, int i, int j)
{
    NewlineMsgRef sendmsg = make_send_msg(ident, i, j);
    send_msg(sync_stream, sendmsg);
    NewlineMsgRef response = recv_msg(sync_stream);
    bool ok = verify(sendmsg, response);
    if(!ok) {
        ctx->error_count++;
    }
    return ok;
}