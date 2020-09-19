#define _GNU_SOURCE
#include <c_eg/worker.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <pthread.h>

#include <c_eg/constants.h>
#include <c_eg/utils.h>
#include <c_eg/socket_functions.h>
#include <c_eg/queue.h>

void socket_read(socket_handle_t socket, void* buffer, int buffer_length)
{
    int how_much = (int)read(socket, buffer, buffer_length);
    
    int saved_errno = errno;
    char* b = (char*)buffer;
    b[how_much] = '\0';
    printf("read socket buffer: \n%s\n", b);

    assert(how_much > 0);
    
}

void socket_write(socket_handle_t socket,  void* buffer, int buffer_length)
{
    int res = (int)write(socket, buffer, buffer_length);
    printf("socket_write res: %d\n", res);
    assert(res >= 0);
    assert(res == buffer_length); // need to handle partial write   
}
void read_message(socket_handle_t socket)
{

}
// void write_message(socket_handle_t socket, MessageRef msg)
// {

// }

void write_string(int socket, char* msg)
{
    socket_write(socket, (void*) msg, strlen(msg));
}

void write_simple_response(int socket)
{
    const char* body = "<html><body><h2>This is a response</h2></body></html>";
    int len = strlen(body);
    char* msg;
    asprintf(&msg, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n%s", len, body);
    printf("response\n%s", msg);
    write_string(socket, msg);
}

void http_handler(int socket)
{
    const int buffer_len = 10000;
    char buffer[buffer_len];
    {
        printf("Inside %s  socket: %d\n", __FUNCTION__, socket);
        socket_read(socket, buffer, buffer_len);
        write_simple_response(socket);
        sleep(5);
    }
}

struct Worker_s {
    bool        active;
    int         active_socket;
    QueueRef    qref;
    pthread_t   pthread;
    int         id;
};

WorkerRef Worker_new(QueueRef qref, int _id)
{
    WorkerRef wref = (WorkerRef)malloc(sizeof(Worker));
    if(wref == NULL)
        return NULL;
    wref->active_socket = 0;
    wref->active  = false;
    wref->id = _id;
    wref->qref = qref;
    printf("returning from Worker_new\n");
    return wref;
}
void Worker_free(WorkerRef wref)
{
}
static void* Worker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    WorkerRef wref = (WorkerRef)data;
    while(true)
    {
        printf("Worker_main start of loop\n");
        wref->active = false;

        int mySocketHandle = Queue_remove(wref->qref);
        wref->active_socket = (int) mySocketHandle;
        wref->active = true;
        // in here read and service a http request
        http_handler(wref->active_socket);
        
        wref->active = false;
    }
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
void Worker_start(WorkerRef wref)
{
    ASSERT_NOT_NULL(wref);

    printf("Worker start - enter\n");
    int rc = pthread_create(&(wref->pthread), NULL, &(Worker_main), (void*) wref);
    printf("Worker start exit\n");
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

