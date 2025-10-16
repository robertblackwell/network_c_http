
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <common/socket_functions.h>
#include <server/server_ctx.h>
#include "../test_ctx.h"
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
