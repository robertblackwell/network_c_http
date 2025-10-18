#include "test_ctx.h"
void testctx_init(TestCtxRef ctx, int port, int id, int nbr_connections, int nbr_messages_per_connection) {
    ctx->port = 9002;
    ctx->id = id;
    ctx->nbr_connections = nbr_connections;
    ctx->nbr_msg_per_connection = nbr_messages_per_connection;
    ctx->error_count = 0;
}
