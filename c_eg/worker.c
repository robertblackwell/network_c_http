#define _GNU_SOURCE
#include <c_eg/worker.h>
#include <c_eg/constants.h>
#include <c_eg/alloc.h>
#include <c_eg/utils.h>
#include <c_eg/socket_functions.h>
#include <c_eg/queue.h>
#include <c_eg/parser.h>
#include <c_eg/reader.h>
#include <c_eg/writer.h>

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
    printf("returning from Worker_new\n");
    return wref;
}
void Worker_free(WorkerRef wref)
{
    free((void*)wref);
}

static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    WorkerRef wref = (WorkerRef)data;
    while(true) {
        printf("Worker_main start of loop\n");
        wref->active = false;
        ParserRef parser_ref = NULL;;
        RdrRef rdr = NULL;
        WrtrRef wrtr = NULL;
        MessageRef request_msg_ref = NULL;

        int mySocketHandle = Queue_remove(wref->qref);
        printf("Worker_main %p mySocketHandle: %d\n", wref, mySocketHandle);
        if(mySocketHandle == -1) {
            /// this is the terminate signal
            printf("Worker_main about to break %p mySocketHandle: %d\n", wref, mySocketHandle);
            break;
        }
        wref->active_socket = (int) mySocketHandle;
        int socket = mySocketHandle;
        wref->active = true;
        if((parser_ref = Parser_new()) == NULL) goto finalize;
        if((rdr = Rdr_new(parser_ref, socket)) == NULL) goto finalize;
        if((wrtr = Wrtr_new(socket)) == NULL) goto finalize;

        while((request_msg_ref = Rdr_read(rdr)) != NULL) {
            printf("Got a request socket: %d pthread_self %ld\n", socket, pthread_self());
            if(0 == wref->handler(request_msg_ref, wrtr)) goto finalize;
            Message_free(&request_msg_ref);
            close(socket);
            socket = 0;
        }

        finalize:
            if(socket != 0) close(socket);
            if(request_msg_ref != NULL) Message_free(&request_msg_ref);
            if(parser_ref != NULL) Parser_free(&parser_ref);
            if(wrtr != NULL) Wrtr_free(&wrtr);
            if(rdr != NULL) Rdr_free(&rdr);
            wref->active = false;
    }
    printf("Worker_main exited main loop %p\n", wref);
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
int Worker_start(WorkerRef wref)
{
    ASSERT_NOT_NULL(wref);

    printf("Worker start - enter\n");
    int rc = pthread_create(&(wref->pthread), NULL, &(Worker_main), (void*) wref);
    if(rc) {
        printf("pthread_create failed\n");
        return rc;
    } else {
        printf("Worker start exit\n");
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
