#define _GNU_SOURCE
#include <c_http/details/worker.h>
#include <c_http/constants.h>
#include <c_http/dsl/alloc.h>
#include <c_http/dsl/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/dsl/queue.h>
#include <c_http/details/ll_parser_types.h>
#include <c_http/api/reader.h>
#include <c_http/api/writer.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <pthread.h>


struct Worker_s {
    bool            active;
    int             active_socket;
    QueueRef        qref;
    pthread_t       pthread;
    int             id;
    HandlerFunction handler;
};

WorkerRef Worker_new(QueueRef qref, int _id, HandlerFunction handler)
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
void Worker_free(WorkerRef wref)
{
    free((void*)wref);
}

void handle_parse_error(MessageRef requestref, WriterRef wrtr)
{
    char* reply = "HTTP/1.1 400 BAD REQUEST \r\nContent-length: 0\r\n\r\n";
    Writer_write_chunk(wrtr, (void*)reply, strlen(reply));
}

static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    WorkerRef wref = (WorkerRef)data;
    bool terminate = false;
    while(!terminate) {
        wref->active = false;
        ReaderRef rdr = NULL;
        WriterRef wrtr = NULL;
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
            RdSocket rdsock = RealSocket(sock);
            if((rdr = Reader_new(rdsock)) == NULL) goto finalize;
            if((wrtr = Writer_new(sock)) == NULL) goto finalize;

            while(1) {
                int rc = Reader_read(rdr, &request_msg_ref);
                if((rc == READER_OK) && (request_msg_ref != NULL)) {
                    if(0 == wref->handler(request_msg_ref, wrtr)) goto finalize;
                    Message_free(&request_msg_ref);
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
            if(request_msg_ref != NULL) Message_free(&request_msg_ref);
            if(wrtr != NULL) Writer_free(&wrtr);
            if(rdr != NULL) Reader_free(&rdr);
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
