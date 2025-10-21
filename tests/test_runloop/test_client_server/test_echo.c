
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <rbl/unittest.h>
#include <common/socket_functions.h>
#include "test_ctx.h"
#include <apps/server_process/server_ctx.h>
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
#if defined(MSG_SELECT_ECHO)
    printf("MSG_SELECT_ECHO test has completed\n");
#elif defined(MSG_SELECT_DEMO)
    printf("MSG_SELECT_DEMO test has completed\n");
#else
    printf("NO MSG_SELECTION set -- test has completed\n");
#endif
    return rc;
}
int test_echo()
{
    int count = 0;
    TestCtx tctx[2];
    testctx_init(&tctx[0], 9002, 11, 4, 4);
    testctx_init(&tctx[1], 9002, 22, 4, 4);
    pthread_t server_thread;
    pthread_t client_1_thread;
    pthread_t client_2_thread;
    pthread_create(&server_thread, NULL, server_thread_function, &(tctx[0]));
    sleep(1);
    pthread_create(&client_1_thread, NULL, client_thread_function, &(tctx[0]));count++;
    pthread_create(&client_2_thread, NULL, client_thread_function, &(tctx[1]));count++;
    pthread_join(client_1_thread, NULL);
    pthread_join(client_2_thread, NULL);
    UT_EQUAL_INT(tctx[0].error_count, 0);
    // UT_EQUAL_INT(tctx[1].error_count, 0);
    pthread_join(server_thread, NULL);
    printf("\nCOMPLETED test_echo complete number servers %d number clients %d\n", 1, count);
    return 0;
}
void* server_thread_function(void* tctx)
{
    static ServerCtx server_ctx;
    TestCtxRef ctx = (TestCtxRef)tctx;
    int port = ctx->port;
    int fd = create_bound_socket(port, "127.0.0.1");
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = &server_ctx;
    server_ctx_init(server_ctx_ref, runloop, fd);
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, 3000L);
    printf("server thread ending htons(9002): %d\n", htons(port));
    return 0;
}
