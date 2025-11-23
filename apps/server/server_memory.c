#include <src/runloop/runloop.h>
#include "server_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <src/common/socket_functions.h>
#include <apps/simple_request_response_app/simple_app.h>
void* server_allocate_app_memory(ServerCtxRef server_ctx)
{
#ifdef SERVER_MEMORY_USE_OBJECT_POOL
    SimpleAppRef app_ref = object_pool_allocate(server_ctx->app_object_pool);
    return app_ref;
#else
    void* app_ref = malloc(sizeof(SimpleApp));
    return app_ref;
#endif
}
void* server_init_app(ServerCtxRef server_ctx, void* app_memory, int new_sock)
{
    RBL_SET_TAG(ServerCtx_TAG, server_ctx)
    RBL_SET_END_TAG(ServerCtx_TAG, server_ctx)
    RunloopRef rl =  server_ctx->runloop_ref;
    assert(app_memory);
    SimpleAppRef app_ref = app_memory;
    simple_app_init(app_ref, rl, new_sock);
    return app_ref;
}
void server_deinit_app(ServerCtxRef ctx, SimpleAppRef app_ptr)
{
    simple_app_deinit(app_ptr);
}
/**
 * free the memory only
 */
void server_dealloc_only_app_memory(ServerCtxRef server, SimpleApp* app_ptr)
{
#ifdef SERVER_MEMORY_USE_OBJECT_POOL
    object_pool_deallocate(server->app_object_pool, app_ptr);
#else
    free(app_ptr);
#endif
}
SimpleAppRef server_provide_init_app(ServerCtxRef ctx, int new_sock)
{
    assert(ctx->pending_app_memory);
    SimpleAppRef app_ref = ctx->pending_app_memory;
    server_init_app(ctx, app_ref, new_sock);
    ctx->pending_app_memory = server_allocate_app_memory(ctx);
    return app_ref;
}
void server_reclaim_app(ServerCtxRef ctx, SimpleAppRef app_ref)
{
    server_deinit_app(ctx, app_ref);
    if (ctx->pending_app_memory == NULL) {
        ctx->pending_app_memory = app_ref;
    } else {
        server_dealloc_only_app_memory(ctx, app_ref);
    }
}
bool server_has_resource_to_accept(ServerCtxRef ctx)
{
    return ctx->pending_app_memory != NULL;
}
