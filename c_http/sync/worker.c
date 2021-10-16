#define _GNU_SOURCE
#include <c_http/sync/worker.h>
#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
#include <c_http/common/queue.h>
#include <c_http/sync/sync_reader.h>
#include <c_http/sync/sync_writer.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>


struct Worker_s {
    bool                active;
    int                 active_socket;
    QueueRef            qref;
    pthread_t           pthread;
    int                 id;
    SyncHandlerFunction handler;
};

WorkerRef Worker_new(QueueRef qref, int _id, SyncHandlerFunction handler)
{
    WorkerRef wref = (WorkerRef)eg_alloc(sizeof(Worker));
    if(wref == NULL)
        return NULL;
    wref->active_socket = 0;
    wref->active  = false;
    wref->id = _id;
    wref->qref = qref;
    wref->handler = handler;
    return wref;
}
void Worker_dispose(WorkerRef wref)
{
    free((void*)wref);
}

void handle_parse_error(MessageRef requestref, SyncWriterRef wrtr)
{
    char* reply = "HTTP/1.1 400 BAD REQUEST \r\nContent-length: 0\r\n\r\n";
    SyncWriter_write_chunk(wrtr, (void*)reply, strlen(reply));
}

static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    WorkerRef wref = (WorkerRef)data;
    bool terminate = false;
    while(!terminate) {
        wref->active = false;
        SyncReaderRef rdr = NULL;
        SyncWriterRef wrtr = NULL;
        MessageRef request_msg_ref = NULL;

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
            if((rdr = SyncReader_new(sock)) == NULL) goto finalize;
            if((wrtr = SyncWriter_new(sock)) == NULL) goto finalize;

            while(1) {
                int rc = SyncReader_read(rdr, &request_msg_ref);
                if((rc == READER_OK) && (request_msg_ref != NULL)) {
                    if(0 == wref->handler(request_msg_ref, wrtr)) goto finalize;
                    Message_dispose(&request_msg_ref);
                    close(sock);
                    sock = 0;
                    break;
                } else if(rc == READER_PARSE_ERROR) {
                    // send a reply bad request
                    handle_parse_error(request_msg_ref, wrtr);
                    break;
                } else {
                    break;
                }
            }
        }

        finalize:
            if(sock != 0) close(sock);
            if(request_msg_ref != NULL) Message_dispose(&request_msg_ref);
            if(wrtr != NULL) SyncWriter_dispose(&wrtr);
            if(rdr != NULL) SyncReader_dispose(&rdr);
            wref->active = false;
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
// void Worker_set_pthread(Workerref wref, pthread_t* pthread)
// {
//     wref->pthread = pthread;
// }
pthread_t* Worker_pthread(WorkerRef wref)
{
    ASSERT_NOT_NULL(wref);
    return &(wref->pthread);
}
void Worker_join(WorkerRef wref)
{
    int retvalue;
    pthread_join(wref->pthread, NULL);
}
