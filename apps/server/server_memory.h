#ifndef H_server_ctx_memory_H
#define H_server_ctx_memory_H
#include "server_ctx.h"
// #include <rbl/unittest.h>
#include <apps/simple_request_response_app/simple_app.h>

/**
 * Allocates raw uninitialized memory for an instance of SimpleApp
 */
void* server_allocate_app_memory(ServerCtxRef ctx);
/**
 * Initializes raw memory into a functional instance of SimpleApp for the new_sock
 */
void* server_init_app(ServerCtxRef server_ctx, void* app_memory, int new_sock);
/**
 * De-Initializes an instance of SimpleApp where necessary deallocating any sub components
 * but does not free() the underlying apps raw emory
 */
void server_deinit_app(ServerCtxRef ctx, SimpleAppRef app_ptr);
/**
 * De-allocates the raw memory that could/used-to hold an instance of SimpleApp.
 * Does not access any of the app fields as assumes they are all undefined.
 */
void server_dealloc_only_app_memory(ServerCtxRef server, SimpleApp* app_ptr);
/**
 * The server provides a full initialized instance of SimpleApp pointer.
 * The server has a caching scheme for this memory to reduce allocation costs.
 */
SimpleAppRef server_provide_init_app(ServerCtxRef ctx, int new_sock);
/**
 * The server de-inits an instance of SimpleApp and frees or reuses the raw memory
 * as reqired by the severs memory caching scheme.
 */
void server_reclaim_app(ServerCtxRef ctx, SimpleAppRef app_ref);
/**
 * Returns true if the servers caching scheme can guarantee that there is enough memory to create a new
 * instance of SimpleApp for the result of a new accept() call. This guarentees that accept() calls are only
 * performed when it is guarenteed the socket arising from that accept() call can be servicced
 * by a new instance of SimpleApp.
 */
bool server_has_resource_to_accept(ServerCtxRef ctx);
#endif