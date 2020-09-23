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

void write_string(int socket, char* msg)
{
    socket_write(socket, (void*) msg, strlen(msg));
}

void write_simple_response(socket_handle_t socket)
{
    const char* body = "<html><body><h2>This is a response</h2></body></html>";
    int len = strlen(body);
    char* msg;
    asprintf(&msg, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: %d\r\n\r\n%s", len, body);
    printf("response\n%s", msg);
    write_string(socket, msg);
}

void http_handler(socket_handle_t socket)
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
    WorkerRef wref = (WorkerRef)eg_alloc(sizeof(Worker));
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
    free((void*)wref);
}
char* simple_response_body(char* message, socket_handle_t socket, int pthread_self_value)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));

    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, message, dt_s, socket, pthread_self_value);

    return s1;
}

char* simple_response(char* message)
{

    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, message);

    char* prefix =  "HTTP/1.1 200 OK\r\n"
                    "content-length: %d\r\n"
                    "\r\n%s";
    char* s2;
    int len2 = asprintf(&s2, prefix, len1, s1);
    free(s1);
    return s2;
}
/**
 * Make a generic response consisting of 200 status, content length header and full request as body
 * @param request
 */
void make_generic_response(MessageRef request)
{
}
int handle_request(MessageRef request, WrtrRef wrtr)
{
    char* msg = "<h2>this is a message</h2>";
    char* body = NULL;
    char* body_len_str = NULL;
    HDRListRef hdrs = NULL;
    HeaderLineRef hl_content_length = NULL;
    HeaderLineRef hl_content_type = NULL;
    int return_value = 0;

    printf("Handle request\n");
    if((body = simple_response_body(msg, wrtr->socket, pthread_self())) == NULL) goto finalize;

    int body_len = strlen(body);
    if(-1 == asprintf(&body_len_str, "%d", body_len)) goto finalize;

    if((hdrs = HDRList_new()) == NULL) goto finalize;

    if((hl_content_length = HeaderLine_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str))) == NULL) goto finalize;

    HDRList_add_front(hdrs, hl_content_length);

    char* content_type = "text/html; charset=UTF-8";
    if((hl_content_type = HeaderLine_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type))) == NULL) goto finalize;

    HDRList_add_front(hdrs, hl_content_type);

    Wrtr_start(wrtr, HTTP_STATUS_OK, hdrs);
    Wrtr_write_chunk(wrtr, (void*) body, body_len);

    return_value = 1;

    finalize:
        if(hdrs != NULL) HDRList_free(&hdrs);
        if(body != NULL) free(body);
        if(body_len_str != NULL) free(body_len_str);
        return return_value;
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
            if(0 == handle_request(request_msg_ref, wrtr)) goto finalize;
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

