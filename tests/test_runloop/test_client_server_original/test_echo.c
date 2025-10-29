
#include <pthread.h>
#ifdef APPLE_FLAG
#include <sys/_pthread/_pthread_t.h>
#endif
#include <rbl/unittest.h>
#include <sync_msg_stream/sync_msg_stream.h>
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
int test_client_server();

int main() {
    printf("test_echo msg selection is : %s\n", generic_msg_selection());
    UT_ADD(test_client_server);
    int rc = UT_RUN();
    return rc;
}
int test_client_server() {
    int nbr_servers = 5;
    int nbr_clients = 10;
    pthread_t server_thread[nbr_servers];
    pthread_t client_thread[nbr_clients];
    TestCtx tctx[nbr_clients];
    for(int iclient = 0; iclient < nbr_clients; iclient++) {
        testctx_init(&tctx[iclient], iclient*10+iclient, 4, 4);
    }
    for (int iserver = 0; iserver < nbr_servers; iserver++) {
        pthread_create(&server_thread[iserver], NULL, server_thread_function, &(tctx[0]));
    }
    sleep(3);
    for (int iclient = 0; iclient < nbr_clients; iclient++) {
        pthread_create(&client_thread[iclient], NULL, client_thread_function, &(tctx[0]));
    }
    for(int iclient = 0; iclient < nbr_clients; iclient++) {
        UT_EQUAL_INT(tctx[iclient].error_count, 0);
        pthread_join(client_thread[iclient], NULL);
    }
    for(int iserver = 0; iserver < nbr_servers; iserver++) {
        pthread_join(server_thread[iserver], NULL);
    }
    printf("After all joins\n");
    return 0;
}
bool verify(GenericMsgRef sendmsg, GenericMsgRef response);
GenericMsgRef make_send_msg(int ident, int i, int j);

void* client_thread_function(void* tctx) {

    TestCtxRef ctx = tctx;
    int port = ctx->port;
    int ident = ctx->id;

    for(int i = 0; i < ctx->nbr_connections; i++) {
        SyncMsgStreamRef sync_stream = sync_msg_stream_new();
        sync_msg_stream_connect(sync_stream, port);
        for (int j = 0; j < ctx->nbr_msg_per_connection; j++) {
            GenericMsgRef sendmsg = make_send_msg(ident, i, j);
            sync_msg_stream_send(sync_stream, sendmsg);
            GenericMsgRef response = sync_msg_stream_recv(sync_stream);
            bool ok = verify(sendmsg, response);
            if(!ok) {
                ctx->error_count++;
            }
            printf("Test response %d id: %d i:%d j:%d\n", (int)ok, ident, i, j);
        }
        sync_msg_stream_free(sync_stream);
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
GenericMsgRef make_send_msg(int ident, int i, int j)
{
    char* send_content_ptr = NULL;
    int sl = asprintf(&send_content_ptr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ Client %d connection: %d msg: %d", ident, i, j);
    assert(sl != -1);
    GenericMsgRef msgref = generic_msg_new();
    IOBufferRef iob = IOBuffer_from_cstring(send_content_ptr);
    generic_msg_set_content(msgref, iob);
    return msgref;
}
bool verify(GenericMsgRef sendmsg, GenericMsgRef response)
{
    const char* send_cstr = IOBuffer_cstr(generic_msg_get_content(sendmsg));
    const char* response_cstr = IOBuffer_cstr(generic_msg_get_content(response));
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
