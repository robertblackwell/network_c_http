#define _GNU_SOURCE
#include <c_http/sync/sync.h>
#include <c_http/sync/sync_internal.h>

#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <c_http/check_tag.h>
static int connection_message_handler(MessageRef request_ref, sync_worker_r context)
{
    sync_worker_r worker_ref = context;
    MessageRef response_ref = worker_ref->app_handler(request_ref, worker_ref);
    if(response_ref != NULL) {
        int rc = sync_connection_write(worker_ref->connection_ptr, response_ref);
    }
    return HPE_OK;
}

sync_worker_r sync_worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler)
{
    sync_worker_r wref = (sync_worker_r)eg_alloc(sizeof(sync_worker_t));
    SET_TAG(SYNC_WORKER_TAG, wref)
    if(wref == NULL)
        return NULL;
    wref->active_socket = 0;
    wref->active  = false;
    wref->id = ident;
    wref->qref = qref;
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

        int my_socket_handle = Queue_remove(wref->qref);
        printf("Worker_main %p mySocketHandle: %d worker %d\n", wref, my_socket_handle, wref->id);
        int sock = my_socket_handle;
        if(my_socket_handle == -1) {
            /// this is the terminate signal
            terminate = true;
            sock = 0;
        } else {
            wref->active_socket = (int) my_socket_handle;
            wref->active = true;
            sync_connection_t* conn = sync_connection_new(sock, wref->read_buffer_size);//, connection_message_handler, wref);
            if((wref->connection_ptr = conn) == NULL) goto finalize;
            int rc = sync_connection_read_request(wref->connection_ptr, connection_message_handler, wref);
        }
        finalize:
            if(sock != 0) close(sock);
            if(wref != NULL) sync_worker_dispose(wref);
    }
    printf("Worker_main exited main loop %p, %d\n", wref, wref->id);
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
