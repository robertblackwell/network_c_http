#define _GNU_SOURCE
#include <c_eg/handler_example.h>

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

int handler_example(Message* request, Writer* wrtr)
{
    char* msg = "<h2>this is a message</h2>";
    char* body = NULL;
    char* body_len_str = NULL;
    HdrList* hdrs = NULL;
    KVPair* hl_content_length = NULL;
    KVPair* hl_content_type = NULL;
    int return_value = 0;

    printf("Handle request\n");
    if((body = simple_response_body(msg, wrtr->m_sock, pthread_self())) == NULL) goto finalize;

    int body_len = strlen(body);
    if(-1 == asprintf(&body_len_str, "%d", body_len)) goto finalize;

    if((hdrs = HdrList_new()) == NULL) goto finalize;

    if((hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str))) == NULL) goto finalize;

    HdrList_add_front(hdrs, hl_content_length);

    char* content_type = "text/html; charset=UTF-8";
    if((hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type))) == NULL) goto finalize;

    HdrList_add_front(hdrs, hl_content_type);

    Writer_start(wrtr, HTTP_STATUS_OK, hdrs);
    Writer_write_chunk(wrtr, (void*) body, body_len);

    return_value = 1;

    finalize:
    if(hdrs != NULL) HdrList_free(&hdrs);
    if(body != NULL) free(body);
    if(body_len_str != NULL) free(body_len_str);
    return return_value;
}
