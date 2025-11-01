#include "async_process_main.h"
#include <apps/server/server_ctx.h>
#include <assert.h>
#include <pthread.h>
#include <runloop/runloop.h>
#include <src/common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <unistd.h>
// #include <mcheck.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>

#define MAX_NBR_THREADS 10
void* thread_function(void* arg);

typedef struct ThreadContext_s {
    int             ident;
    pthread_t       thread;
    int             port;
    const char*     host;
    int             listening_socket;
    void*           return_value;
    ServerCtxRef    server_ref;
} ThreadContext;


void process_main(char* host, int port, int nbr_threads, int nbr_connections_per_thread, int nbr_rountrips_per_connection)
{
    ThreadContext thread_table[nbr_threads];
    assert(nbr_threads <= MAX_NBR_THREADS);
    printf("Process starting pid: %d\n", getpid());
    for(int i = 0; i < nbr_threads; i++) {
        ThreadContext* ctx = &(thread_table[i]);
        ctx->ident = i;
        ctx->port = port;
        ctx->host = host;
        pthread_create(&(ctx->thread), NULL, thread_function, ctx);
    }
    for(int i = 0; i < nbr_threads; i++) {
        ThreadContext* ctx = &(thread_table[i]);
        pthread_join(ctx->thread, (void**)&(ctx->return_value));
    }
}
void* thread_function(void* arg)
{
    ServerCtx server_ctx;
    ThreadContext* ctx = arg;
    int fd = create_bound_socket(ctx->port, ctx->host);
    printf("thread pid: %d tid: %lu host: %s port: %d ident: %d pthread_t: %lu fd: %d\n", getpid(), (unsigned long)pthread_self(), ctx->host, ctx->port, ctx->ident, (unsigned long)ctx->thread, ctx->listening_socket);
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = &server_ctx;
    server_ctx_init(server_ctx_ref, runloop, fd);
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, -1L);
    return NULL;
}
