#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>
#include <c_eg/list.h>
#include <c_eg/server.h>
#include <c_eg/headerline_list.h>
#include <c_eg/message.h>
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
typedef struct X_s {
    socket_handle_t socket;
} X, *XRef;
X wrtr_s = {42};
XRef wrtr = &wrtr_s;

void Wrtr_start(HttpStatus status, HDRListRef headers)
{
    const char* reason_str = http_status_str(status);
    char* first_line = NULL;
    int len = asprintf(&first_line, "HTTP/1.1 %d %s\r\n", status, reason_str);
    if(first_line == NULL) goto failed;

    CBufferRef cb_output_ref = NULL;
    if((cb_output_ref = CBuffer_new()) == NULL) goto failed;
    CBufferRef serialized_headers = NULL;
    serialized_headers = HDRList_serialize(headers);

    CBuffer_append(cb_output_ref, (void*)first_line, len);
    /// this is clumsy - change HDRList_serialize() to deposit into an existing ContigBuffer
    CBuffer_append(cb_output_ref, CBuffer_data(serialized_headers), CBuffer_size(serialized_headers));
    CBuffer_append_cstr(cb_output_ref, "\r\n");
    int x = len+2;

    free(first_line);
    CBuffer_free(&serialized_headers);
    CBuffer_free(&cb_output_ref);
    return;
    failed:
    if(first_line != NULL) free(first_line);
    if(serialized_headers != NULL) CBuffer_free(&serialized_headers);
    if(cb_output_ref != NULL) CBuffer_free(&cb_output_ref);

}


int test_handle_request()
{
    printf("Handle request\n");
    char* msg = "<h2>this is a message</h2>";
    char* body = simple_response_body(msg, wrtr->socket, pthread_self());
    int body_len = strlen(body);
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);
    HDRListRef hdrs = HDRList_new();
    HeaderLineRef hl_content_length = HeaderLine_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HDRList_add_front(hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    HeaderLineRef hl_content_type = HeaderLine_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HDRList_add_front(hdrs, hl_content_type);

//    Wrtr_start(wrtr, HTTP_STATUS_OK, hdrs);
//    Wrtr_write_chunk(wrtr, (void*) body, body_len);

    HDRList_free(&hdrs);
//    HeaderLine_free(&(hl_content_length));
//    HeaderLine_free(&(hl_content_type));
    free(body);
    free(body_len_str);

    return 0;
}
int main()
{
    UT_ADD(test_handle_request);
    int rc = UT_RUN();
    return rc;
}