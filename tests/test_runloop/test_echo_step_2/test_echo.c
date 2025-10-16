
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <rbl/unittest.h>
#include "test_ctx.h"
#include <server/server_ctx.h>
#include <client/client_thread_function.h>
/**
* This program tests the echo application and newline_msg protocal in a single process. So that is can be used
* as part of a unit test suite.
*
* The server uses much of the machinery of apps/tcp apps/server_apps apps.async_msg_stream
* but the client part is entirely built just for this test and uses no general machinery
*/

void* server_thread_function(void* tctx);
int test_echo();

int main() {
    UT_ADD(test_echo);
    int rc = UT_RUN();
    return rc;
}
int test_echo() {
    TestCtx tctx[2];
    testctx_init(&tctx[0], 9002, 11, 4, 4);
    testctx_init(&tctx[1], 9002, 22, 4, 4);
    pthread_t server_thread;
    pthread_t client_1_thread;
    pthread_t client_2_thread;
    pthread_create(&server_thread, NULL, server_thread_function, &(tctx[0]));
    sleep(3);
    // pthread_create(&client_1_thread, NULL, client_thread_function, &(tctx[0]));
    pthread_create(&client_2_thread, NULL, client_thread_function, &(tctx[1]));
    pthread_join(client_1_thread, NULL);
    // pthread_join(client_2_thread, NULL);
    UT_EQUAL_INT(tctx[0].error_count, 0);
    UT_EQUAL_INT(tctx[1].error_count, 0);
    pthread_join(server_thread, NULL);
    return 0;
}
#if 0
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
            char send_msg[200] = "";
            sprintf(send_msg, "Client %d connection: %d msg: %d", ctx->id, i, j);
            char send_buffer[200];
            sprintf(send_buffer, "%s\n", send_msg);
            size_t lenx = strlen(send_buffer);
            printf("sending len:%lu msg:%s", lenx, send_buffer);
            send(lfd, send_buffer, lenx, 0);
            char recieve_buffer[200];
            recv(lfd, recieve_buffer, sizeof recieve_buffer, 0);
            printf("[server] %s", recieve_buffer);
            char test_buffer[200];
            sprintf(test_buffer, "ServerResponse:[%s]\n\n", send_msg);
            int ok = strcmp(test_buffer, recieve_buffer);
            if(ok != 0) {
                ctx->error_count++;
            }
            printf("Test response %d id: %d i:%d j:%d\n", ok, ctx->id, i, j);
        }
        close(lfd);
    }
    return NULL;
}
#endif
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
