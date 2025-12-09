#include "verify_thread_context.h"
#include <stdlib.h>
#include <assert.h>

VerifyThreadContext* verify_ctx_new(int port,
    int process_id_nbr, int thread_id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread)
{
    VerifyThreadContext* ctx = malloc(sizeof(VerifyThreadContext));
    verify_ctx_init(ctx, port, process_id_nbr, thread_id, nbr_roundtrips_per_connection, nbr_connections_per_thread);
    return ctx;
}
void verify_ctx_init(VerifyThreadContextRef ctx, int port,
    int process_id_nbr, int thread_id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread)
{
    if (ctx == NULL) assert(0);
    ctx->port = port;
    ctx->process_nbr = process_id_nbr;
    ctx->max_rountrips_per_connection = nbr_roundtrips_per_connection;
    ctx->max_connections_per_thread = nbr_connections_per_thread;
    int n = nbr_roundtrips_per_connection * nbr_connections_per_thread;
    ctx->response_times_ref = rta_new(n);
    ctx->ident = thread_id;
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
void verify_ctx_reset_connection_round_trip_count(VerifyThreadContextRef ctx)
{
    ctx->roundtrip_per_connection_counter = 0;
}
