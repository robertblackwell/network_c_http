#include "demo_sync_process_main.h"

#include <http_in_c/demo_protocol/demo_sync_socket.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/common/socket_functions.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <stdio.h>
#include <errno.h>
#include <mcheck.h>
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
} ThreadContext;

void server_loop(int listen_socket);
DemoMessageRef process_request(DemoMessageRef);

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
    ThreadContext* ctx = arg;
    int listening_socket_fd = create_listener_socket(ctx->port, ctx->host);
    printf("thread pid: %d tid: %d host: %s port: %d listening_socket: %d\n", getpid(), gettid(), ctx->host, ctx->port, listening_socket_fd);

    server_loop(listening_socket_fd);

    return NULL;
}

void server_loop(int listen_socket)
{
    bool terminate = false;
    while(!terminate) {
        struct sockaddr peername;
        socklen_t addr_length;
        int sock2 = accept(listen_socket, (struct sockaddr*)&peername, &addr_length);
        if( sock2 <= 0 ){
            RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
            int en = errno;
            break;
        }
        RBL_LOG_FMT("Worker_main %p mySocketHandle: %d worker %d ########################################################################### %d START", wref, my_socket_handle, wref->id, wref->id);
        DemoSyncSocketRef connection_ref = demo_syncsocket_new_from_fd(sock2);
        DemoMessageRef request_ptr = NULL;
        while(1) {
            int retcode = demo_syncsocket_read_message(connection_ref, &request_ptr);
            if((retcode < 0) || (request_ptr == NULL)) {
                break;
            }
            DemoMessageRef response_ptr = demo_message_new();
            demo_process_request(NULL, request_ptr, response_ptr);
            IOBufferRef iob_req = demo_message_serialize(request_ptr);
            IOBufferRef iob_resp = demo_message_serialize(response_ptr);
            IOBuffer_free(iob_req);
            IOBuffer_free(iob_resp);
            demo_message_free(request_ptr);
            request_ptr = NULL;
            assert(response_ptr != NULL);
            demo_syncsocket_write_message(connection_ref, response_ptr);
            demo_message_free(response_ptr);
            response_ptr = NULL;
        }
        demo_syncsocket_close(connection_ref);
        demo_syncsocket_free(connection_ref);
    }
    RBL_LOG_FMT("Worker_main exited main loop %p, %d", wref, wref->id);
}
