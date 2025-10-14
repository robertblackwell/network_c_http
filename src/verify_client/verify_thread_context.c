#include "verify_thread_context.h"

VerifyThreadContext* verify_ctx_new(int port, int id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread, int max_threads)
{
    VerifyThreadContext* ctx = malloc(sizeof(VerifyThreadContext));
    verify_ctx_init(ctx, port, id, nbr_roundtrips_per_connection, nbr_connections_per_thread, max_threads);
    return ctx;
}
void verify_ctx_init(VerifyThreadContextRef ctx, int port, int id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread, int max_threads)
{
    if (ctx == NULL) assert(0);
    ctx->port = port;
    ctx->max_rountrips_per_connection = nbr_roundtrips_per_connection;
    ctx->max_connections_per_thread = nbr_connections_per_thread;
    int n = nbr_roundtrips_per_connection * nbr_connections_per_thread;
    ctx->response_times_ref = rta_new(n);
    ctx->ident = id;
    ctx->roundtrip_per_connection_counter = 0;
    ctx->total_roundtrips = 0;
}
void verify_ctx_increment_total_round_trip_count(VerifyThreadContextRef ctx)
{
    ctx->total_roundtrips++;
}
void verify_ctx_increment_connection_round_trip_count(VerifyThreadContextRef ctx)
{
    ctx->roundtrip_per_connection_counter++;
}
