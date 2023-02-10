
#define CHLOG_ON
#include <http_in_c/sync/sync.h>
#include <http_in_c/sync/sync_internal.h>

#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/queue.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <http_in_c/macros.h>
#include <http_in_c/logger.h>
#include <http_in_c/check_tag.h>
static int connection_message_handler(MessageRef request_ptr, sync_worker_r context)
{
    sync_worker_t* worker_ptr = context;
    MessageRef response_ptr = worker_ptr->app_handler(request_ptr, worker_ptr);
    int cmp_tmp = Message_cmp_header(request_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    if(cmp_tmp == 1) {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
    } else {
        Message_add_header_cstring(response_ptr, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
    }
    CHTTP_ASSERT((response_ptr != NULL), "response is not permitted to be NULL");
    IOBufferRef request_serialized = Message_dump(request_ptr);
    IOBufferRef response_serialized = Message_dump(response_ptr);
    LOG_FMT("app_handler_example request  ====================================================================");
//    LOG_FMT("%s", IOBuffer_cstr(request_serialized));
//    LOG_FMT("app_handler_example response ====================================================================");
//    LOG_FMT("%s", IOBuffer_cstr(response_serialized));
//    LOG_FMT("app_handler_example end ====================================================================");
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
    SET_TAG(SYNC_WORKER_TAG, wref)
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
    CHECK_TAG(SYNC_WORKER_TAG, wref)
    bool terminate = false;
    while(!terminate) {
        wref->active = false;
        wref->connection_ptr  = NULL;
#if SYNC_WORKER_QUEUE
        int my_socket_handle = Queue_remove(wref->qref);
        wref->working = true;
#else
        struct sockaddr peername;
        socklen_t addr_length;
        int sock2 = accept(wref->listen_socket, (struct sockaddr*)&peername, &addr_length);
        if( sock2 <= 0 )
        {
            LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
            break;
        }
        int my_socket_handle = sock2;
#endif
        LOG_FMT("Worker_main %p mySocketHandle: %d worker %d ########################################################################### %d START", wref, my_socket_handle, wref->id, wref->id);
        int sock = my_socket_handle;
        if(my_socket_handle == -1) {
            /// this is the terminate signal
            terminate = true;
            sock = 0;
        } else {
            wref->active_socket = (int) my_socket_handle;
            wref->active = true;
            sync_connection_t* conn = sync_connection_new(sock, wref->read_buffer_size);//, connection_message_handler, wref);
            wref->connection_ptr = conn;
            int rc = sync_connection_read_request(wref->connection_ptr, connection_message_handler, wref);
            LOG_FMT("worker_main after sync_connection_read_request worker_id: %d#########################################################%d END", wref->id, wref->id);
            wref->working = false;
        }
    }
    LOG_FMT("Worker_main exited main loop %p, %d", wref, wref->id);
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
int sync_worker_start(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    CHECK_TAG(SYNC_WORKER_TAG, wref)

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
    CHECK_TAG(SYNC_WORKER_TAG, wref)
    return wref->active_socket;
}
pthread_t sync_worker_pthread(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    CHECK_TAG(SYNC_WORKER_TAG, wref)
    return wref->pthread;
}
void sync_worker_join(sync_worker_r wref)
{
    ASSERT_NOT_NULL(wref);
    CHECK_TAG(SYNC_WORKER_TAG, wref)
    int retvalue;
    pthread_join(wref->pthread, NULL);
}
