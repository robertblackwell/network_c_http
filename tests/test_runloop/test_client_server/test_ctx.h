#ifndef TEST_ECHO_STEP_2_TEST_CTX_H
#define TEST_ECHO_STEP_2_TEST_CTX_H
typedef struct TestCtx {
    int     port;
    int     id;
    int     nbr_connections;
    int     nbr_msg_per_connection;
    int error_count;
} TestCtx, *TestCtxRef;

void testctx_init(TestCtxRef ctx, int port, int id, int nbr_connections, int nbr_messages_per_connection);
#endif