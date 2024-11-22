#include "http_sync_process_main.h"

#include <http_in_c/http_protocol/http_sync_socket.h>
#include <http_in_c/http_protocol/http_message.h>
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
#include "http_common/http_make_request_response.h"

#define MAX_NBR_THREADS 10
void* thread_function(void* arg);

typedef struct SyncThreadContext_s {
    int             ident;
    pthread_t       thread;
    int             port;
    const char*     host;
    int             listening_socket;
    void*           return_value;
} SyncThreadContext;

void server_loop(int listen_socket);

void process_main(char* host, int port, int nbr_threads, int nbr_connections_per_thread, int nbr_rountrips_per_connection)
{
    SyncThreadContext thread_table[nbr_threads];
    assert(nbr_threads <= MAX_NBR_THREADS);
    printf("Process starting pid: %d\n", getpid());
    for(int i = 0; i < nbr_threads; i++) {
        SyncThreadContext* ctx = &(thread_table[i]);
        ctx->ident = i;
        ctx->port = port;
        ctx->host = host;
        pthread_create(&(ctx->thread), NULL, thread_function, ctx);
    }
    for(int i = 0; i < nbr_threads; i++) {
        SyncThreadContext* ctx = &(thread_table[i]);
        pthread_join(ctx->thread, (void**)&(ctx->return_value));
    }
}
void* thread_function(void* arg)
{
    SyncThreadContext* ctx = arg;
    int listening_socket_fd = create_listener_socket(ctx->port, ctx->host);
    int x = socket_is_blocking(listening_socket_fd);
    printf("thread pid: %d tid: %d host: %s port: %d ident: %d listening_socket: %d\n", getpid(), gettid(), ctx->host, ctx->port, ctx->ident, listening_socket_fd);

    server_loop(listening_socket_fd);

    return NULL;
}

void server_loop(int listen_socket)
{
    bool terminate = false;
    while(!terminate) {
        struct sockaddr peername;
        socklen_t addr_length = 0;
        int sock2 = accept(listen_socket, (struct sockaddr*)&peername, &addr_length);
        if( sock2 <= 0 ){
            RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
            int en = errno;
            break;
        }
        RBL_LOG_FMT("Worker_main %p mySocketHandle: %d worker %d ########################################################################### %d START", wref, my_socket_handle, wref->id, wref->id);
        HttpSyncSocketRef connection_ref = http_syncsocket_new_from_fd(sock2);
        HttpMessageRef request_ptr = NULL;
        while(1) {
            int retcode = http_syncsocket_read_message(connection_ref, &request_ptr);
            if((retcode < 0) || (request_ptr == NULL)) {
                break;
            }
            HttpMessageRef response_ptr = http_message_new();
            http_process_request(NULL, request_ptr, response_ptr);
            IOBufferRef iob_req = http_message_serialize(request_ptr);
            IOBufferRef iob_resp = http_message_serialize(response_ptr);
            IOBuffer_free(iob_req);
            IOBuffer_free(iob_resp);
            http_message_free(request_ptr);
            request_ptr = NULL;
            assert(response_ptr != NULL);
            http_syncsocket_write_message(connection_ref, response_ptr);
            http_message_free(response_ptr);
            response_ptr = NULL;
        }
        http_syncsocket_close(connection_ref);
        http_syncsocket_free(connection_ref);
    }
    RBL_LOG_FMT("Worker_main exited main loop %p, %d", wref, wref->id);
}
