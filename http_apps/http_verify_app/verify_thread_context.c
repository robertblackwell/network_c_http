//
// Created by robert on 11/10/24.
//

#include "verify_thread_context.h"

//struct ThreadContext_s {
//    /**
//     * How many round trips in this experiment
//     */
//    int max_rountrips_per_connection;
//    int rountrips_count;
//
//    int max_connections_per_thread;
//    /**
//     * Unique identifier for this thread
//     */
//    int ident;
//    /**
//     * count of roundtrips completed
//     */
//    int roundtrip_per_connection_counter;
//    int total_roundtrips;
//    /**
//     * Total time for this thread
//     */
//    double total_time;
//    /**
//     * response time for each round trip executed by this thread
//     */
//    double resp_times[MAX_ROUNDTRIPS_PER_THREAD];
//    /**
//     * The most recent unique id string generated for round trip
//     */
//    char uid[100];
//};

VerifyThreadContext* Ctx_new(int id, int nbr_roundtrips_per_connection, int nbr_connections_per_thread, int max_threads)
{
    VerifyThreadContext* ctx = malloc(sizeof(VerifyThreadContext));
    ctx->max_rountrips_per_connection = nbr_roundtrips_per_connection;
    ctx->max_connections_per_thread = nbr_connections_per_thread;
    int n = nbr_roundtrips_per_connection * nbr_connections_per_thread;
    ctx->response_times_ref = rta_new(n);
    ctx->ident = id;
    ctx->roundtrip_per_connection_counter = 0;
    ctx->total_roundtrips = 0;
    return ctx;
}

