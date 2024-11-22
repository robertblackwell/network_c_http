#include "tmpl_verify_thread_context.h"

ThreadContext* Ctx_new(int id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread, int max_threads)
{
    ThreadContext* ctx = malloc(sizeof(ThreadContext));
    ctx->max_rountrips_per_connection = nbr_roundtrips_per_connection;
    ctx->max_connections_per_thread = nbr_connections_per_thread;
    int n = nbr_roundtrips_per_connection * nbr_connections_per_thread;
    ctx->response_times_ref = rta_new(n);
    ctx->ident = id;
    ctx->roundtrip_per_connection_counter = 0;
    ctx->total_roundtrips = 0;
    return ctx;
}
