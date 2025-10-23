
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
    return 0;
}
void* client_thread_function(void* tctx) {

    TestCtxRef ctx = tctx;
    int port = ctx->port;
    for(int i = 0; i < ctx->nbr_connections; i++) {
        struct sockaddr_in server;
        int lfd = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = port;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        const int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
        assert(status == 0);
        for (int j = 0; j < ctx->nbr_msg_per_connection; j++) {
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
            // free(send_buff_ptr);
            // free(send_msg_ptr);
            if(ok != 0) {
                ctx->error_count++;
            }
            printf("Test response %d id: %d i:%d j:%d\n", ok, ctx->id, i, j);
        }
        close(lfd);
    }
    return NULL;
}
void* server_thread_function(void* tctx)
{
    static ServerCtx server_ctx;
    TestCtxRef ctx = (TestCtxRef)tctx;
    int port = ctx->port;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = &server_ctx;
    server_ctx_init(server_ctx_ref, runloop, fd);
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, 5000L);
    return 0;
}
