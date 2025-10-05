#include "demo_async_process_main.h"

#include <src/demo_protocol/demo_server.h>
#include <src/demo_protocol/demo_message.h>
#include <src/common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <unistd.h>
// #include <mcheck.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include "demo_common/demo_make_request_response.h"

#define MAX_NBR_THREADS 10
void* thread_function(void* arg);

typedef struct ThreadContext_s {
    int             ident;
    pthread_t       thread;
    int             port;
    const char*     host;
    int             listening_socket;
    void*           return_value;
    DemoServerRef   server_ref;
} ThreadContext;


void demo_process_main(char* host, int port, int nbr_threads, int nbr_connections_per_thread, int nbr_rountrips_per_connection)
{
    ThreadContext thread_table[nbr_threads];
    assert(nbr_threads <= MAX_NBR_THREADS);
    printf("Process starting pid: %p\n", pthread_self());
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
    ThreadContext* ctx = arg;
    int listening_socket_fd = create_listener_socket(ctx->port, ctx->host);
    printf("thread pid: %d tid: %p host: %s port: %d ident: %d pthread_t: %p listening_socket: %d\n", getpid(), pthread_self(), ctx->host, ctx->port, ctx->ident, ctx->thread, ctx->listening_socket);
    ctx->server_ref = demo_server_new(ctx->port, ctx->host, listening_socket_fd, demo_process_request);
    demo_server_listen(ctx->server_ref);
    runloop_run(ctx->server_ref->runloop_ref, -1 /* infinite*/);
    demo_server_free(ctx->server_ref);
    ctx->server_ref = NULL;
    return NULL;
}
