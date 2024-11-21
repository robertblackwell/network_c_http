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
TmplMessageRef mk_request(ThreadContext* ctx)
{
    TmplMessageRef request = tmpl_message_new();
    tmpl_message_set_is_request(request, true);
    BufferChainRef body = BufferChain_new();
    char buf[100];
    sprintf(buf, "%d %d 1234567890", ctx->ident, ctx->roundtrip_per_connection_counter);
    BufferChain_append_cstr(body, buf);
    tmpl_message_set_body(request, body);
    return request;
}

void Ctx_mk_uid(ThreadContext* ctx)
{
    sprintf(ctx->uid, "%d:%d", ctx->ident, ctx->roundtrip_per_connection_counter);
}

/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       ThreadContext*
 * \param request   TmplMessageRef
 * \param response  TmplMessageRef
 * \return bool
 */
bool verify_response(ThreadContext* ctx, TmplMessageRef request, TmplMessageRef response)
{
    BufferChainRef body = tmpl_message_get_body(response);
    IOBufferRef body_iob = BufferChain_compact(body);
    const char* cstr = IOBuffer_cstr(body_iob);
    return true;
//    CbufferRef req_bc = http_message_serialize(request);
//    int x = strcmp(Cbuffer_cstr(body_bc), Cbuffer_cstr(req_bc));
//    if( x != 0) {
//        printf("Verify failed \n");
//        printf("Req     :  %s\n", Cbuffer_cstr(req_bc));
//        printf("Rsp body:  %s\n", Cbuffer_cstr(body_bc));
//    }
//    return (x == 0);
}
