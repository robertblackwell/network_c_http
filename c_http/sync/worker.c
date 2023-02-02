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

//struct Worker_s {
//    bool                active;
//    int                 active_socket;
//    sync_connection_t*  connection_ptr;
//    QueueRef            qref;
//    pthread_t           pthread;
//    int                 id;
//    size_t              read_buffer_size;
//    SyncAppMessageHandler app_handler;
//};
static void connection_message_handler(MessageRef request_ref, WorkerRef context)
{
    WorkerRef worker_ref = context;
    MessageRef response_ref = worker_ref->app_handler(request_ref, worker_ref);
    if(response_ref != NULL) {
        int rc = sync_connection_write(worker_ref->connection_ptr, response_ref);
    }
}

WorkerRef Worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler)
{
    WorkerRef wref = (WorkerRef)eg_alloc(sizeof(Worker));
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
void Worker_dispose(WorkerRef wref)
{
    free((void*)wref);
}

static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    WorkerRef wref = (WorkerRef)data;

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
            sync_connection_t* conn = sync_connection_new(sock, wref->read_buffer_size, connection_message_handler, wref);
            if((wref->connection_ptr = conn) == NULL) goto finalize;

            int rc = sync_connection_read(wref->connection_ptr);
        }

        finalize:
            if(sock != 0) close(sock);
            if(wref != NULL) Worker_dispose(wref);
    }
    printf("Worker_main exited main loop %p, %d\n", wref, wref->id);
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
int Worker_start(WorkerRef wref)
{
    ASSERT_NOT_NULL(wref);

    int rc = pthread_create(&(wref->pthread), NULL, &(Worker_main), (void*) wref);
    if(rc) {
        return rc;
    } else {
        return rc;
    }
}
pthread_t Worker_pthread(WorkerRef wref)
{
    ASSERT_NOT_NULL(wref);
    return wref->pthread;
}
void Worker_join(WorkerRef wref)
{
    int retvalue;
    pthread_join(wref->pthread, NULL);
}
