#include <src/sync/sync.h>
#include <src/sync/sync_internal.h>

#include <src/common/alloc.h>
#include <src/common/utils.h>
#include <src/common/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <rbl/macros.h>
#include <rbl/logger.h>
#include <rbl/check_tag.h>
static int on_message_handler_2(HttpMessageRef response_ptr, sync_worker_t* context)
{
    sync_worker_t* worker_ptr = context;
    RBL_LOG_FMT("verify on_message_handler socket: %d", worker_ptr->connection_ptr->socketfd);
//    RBL_LOG_FMT("verify handler howmany_requests_per_connection: %d", ctx->howmany_requests_per_connection);
//    RBL_LOG_FMT("verify handler howmany_connections: %d", ctx->howmany_connections);
//    worker_ptr->reqsponse_ptr = response_ptr;
    return HPE_OK;
}

static int connection_message_handler(HttpMessageRef request_ptr, sync_worker_r context)
{
    sync_worker_t* worker_ptr = context;
    HttpMessageRef response_ptr = worker_ptr->app_handler(request_ptr, worker_ptr);
    int cmp_tmp = http_message_cmp_header(request_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    if(cmp_tmp == 1) {
        http_message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    } else {
        http_message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
    }
    RBL_ASSERT((response_ptr != NULL), "response is not permitted to be NULL");
    IOBufferRef request_serialized = http_message_dump(request_ptr);
    IOBufferRef response_serialized = http_message_dump(response_ptr);
    RBL_LOG_FMT("app_handler_example request  ====================================================================");
//    RBL_LOG_FMT("%s", IOBuffer_cstr(request_serialized));
//    RBL_LOG_FMT("app_handler_example response ====================================================================");
//    RBL_LOG_FMT("%s", IOBuffer_cstr(response_serialized));
//    RBL_LOG_FMT("app_handler_example end ====================================================================");
    IOBuffer_free(request_serialized);
    IOBuffer_free(response_serialized);

    int rc = sync_connection_write(worker_ptr->connection_ptr, response_ptr);
    if(cmp_tmp != 1) {
        sync_connection_close(worker_ptr->connection_ptr);
    }
    return HPE_OK;
}
#ifdef SYNC_WORKER_QUEUE
sync_worker_r sync_worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler)
#else
sync_worker_r sync_worker_new(int listen_socket, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler)
#endif
{
    sync_worker_r wref = (sync_worker_r)eg_alloc(sizeof(sync_worker_t));
    RBL_SET_TAG(SYNC_WORKER_TAG, wref)
    if(wref == NULL)
        return NULL;
    wref->active_socket = 0;
    wref->active  = false;
    wref->id = ident;
#ifdef SYNC_WORKER_QUEUE
    wref->qref = qref;
#else
    wref->listen_socket = listen_socket;
#endif
    wref->read_buffer_size = read_buffer_size;
    wref->app_handler = app_handler;
    return wref;
}
void sync_worker_dispose(sync_worker_r wref)
{
    free((void*)wref);
}

static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    sync_worker_r wref = (sync_worker_r)data;
    RBL_CHECK_TAG(SYNC_WORKER_TAG, wref)
    bool terminate = false;
    while(!terminate) {
        wref->active = false;
        wref->connection_ptr  = NULL;
        struct sockaddr peername;
        socklen_t addr_length;
        int sock2 = accept(wref->listen_socket, (struct sockaddr*)&peername, &addr_length);
        if( sock2 <= 0 )
        {
            RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
            break;
        }
        int my_socket_handle = sock2;
        RBL_LOG_FMT("Worker_main %p mySocketHandle: %d worker %d ########################################################################### %d START", wref, my_socket_handle, wref->id, wref->id);
        int sock = my_socket_handle;
        if(my_socket_handle == -1) {
            /// this is the terminate signal
            terminate = true;
            sock = 0;
        } else {
            wref->active_socket = (int) my_socket_handle;
            HttpMessageRef request_ptr = NULL;
            wref->active = true;
            sync_connection_t* conn = sync_connection_new(sock, wref->read_buffer_size);//, connection_message_handler, wref);
            wref->connection_ptr = conn;
            while(1) {
                int gotone = sync_connection_read_message(wref->connection_ptr, &request_ptr);
                if (gotone) {
                    RBL_ASSERT((request_ptr != NULL), "request should not be NULL");
                    sync_worker_t *worker_ptr = wref;
                    HttpMessageRef response_ptr = worker_ptr->app_handler(request_ptr, worker_ptr);
                    int cmp_tmp = http_message_cmp_header(request_ptr, HEADER_CONNECTION_KEY,
                                                         HEADER_CONNECTION_KEEPALIVE);
                    if (cmp_tmp == 1) {
                        http_message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY,
                                                        HEADER_CONNECTION_KEEPALIVE);
                    } else {
                        http_message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
                    }
                    RBL_ASSERT((response_ptr != NULL), "response is not permitted to be NULL");
                    IOBufferRef request_serialized = http_message_dump(request_ptr);
                    IOBufferRef response_serialized = http_message_dump(response_ptr);
                    RBL_LOG_FMT("app_handler_example request  ====================================================================");
                    RBL_LOG_FMT("%s", IOBuffer_cstr(request_serialized));
                    RBL_LOG_FMT("app_handler_example response ====================================================================");
                    RBL_LOG_FMT("%s", IOBuffer_cstr(response_serialized));
                    RBL_LOG_FMT("app_handler_example end ====================================================================");
                    IOBuffer_free(request_serialized);
                    IOBuffer_free(response_serialized);
                    int rc = sync_connection_write(worker_ptr->connection_ptr, response_ptr);
                    if (cmp_tmp != 1) {
                        break;
                    }
                } else {
                    break;
                }
            }
            sync_connection_dispose(&wref->connection_ptr);
            RBL_LOG_FMT("worker_main after sync_connection_read_request worker_id: %d#########################################################%d END", wref->id, wref->id);
            wref->working = false;
        }
    }
    RBL_LOG_FMT("Worker_main exited main loop %p, %d", wref, wref->id);
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
int sync_worker_start(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    RBL_CHECK_TAG(SYNC_WORKER_TAG, wref)

    int rc = pthread_create(&(wref->pthread), NULL, &(Worker_main), (void*) wref);
    if(rc) {
        return rc;
    } else {
        return rc;
    }
}
int sync_worker_socketfd(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    RBL_CHECK_TAG(SYNC_WORKER_TAG, wref)
    return wref->active_socket;
}
pthread_t sync_worker_pthread(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    RBL_CHECK_TAG(SYNC_WORKER_TAG, wref)
    return wref->pthread;
}
void sync_worker_join(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    RBL_CHECK_TAG(SYNC_WORKER_TAG, wref)
    int retvalue;
    pthread_join(wref->pthread, NULL);
}
