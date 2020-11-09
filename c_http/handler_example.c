#define _GNU_SOURCE
#include <c_http/handler_example.h>

#include <c_http/worker.h>
#include <c_http/constants.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/queue.h>
#include <c_http/parser.h>
#include <c_http/reader.h>
#include <c_http/writer.h>

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
/**
 * Create the body of a response to an /echo request
 * \param request
 * \return char* ownership, and responsibility to free, transfers to the caller
 */
char* echo_body(MessageRef request)
{
    CbufferRef cb_body = Message_serialize(request);
    char* body = malloc(Cbuffer_size(cb_body) + 1);
    memcpy(body, Cbuffer_data(cb_body), Cbuffer_size(cb_body)+1);
    return body;
}
int handler_example(MessageRef request, WriterRef wrtr)
{
    char* msg = "<h2>this is a message</h2>";
    char* body = NULL;
    char* body_len_str = NULL;
    HdrListRef resp_hdrs = NULL;
    KVPairRef hl_content_length = NULL;
    KVPairRef hl_content_type = NULL;
    int return_value = 0;

    if((resp_hdrs = HdrList_new()) == NULL) goto finalize;

    CbufferRef target = Message_get_target(request);
    char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        HdrListRef req_hdrs = Message_headers(request);

        /**
         * find the echo-id header in the request and put it in the response headers
         */
        KVPairRef kvp = HdrList_find(req_hdrs, HEADER_ECHO_ID);
        assert(kvp != NULL);
        HdrList_add_cstr(resp_hdrs, HEADER_ECHO_ID, KVPair_value(kvp));

        body = echo_body(request);
    } else {
        if((body = simple_response_body(msg, wrtr->m_sock, pthread_self())) == NULL) goto finalize;
    }
    int body_len = strlen(body);
    if(-1 == asprintf(&body_len_str, "%d", body_len)) goto finalize;


    if(true) {
        if((hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str))) == NULL) goto finalize;
        HdrList_add_front(resp_hdrs, hl_content_length);
        char* content_type = "text/html; charset=UTF-8";
        if((hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type))) == NULL) goto finalize;
        HdrList_add_front(resp_hdrs, hl_content_type);
    } else {
        HdrList_add_cstr(resp_hdrs, HEADER_CONTENT_LENGTH, body_len_str);
        HdrList_add_cstr(resp_hdrs, HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");
    }
    HdrList_add_cstr(resp_hdrs, "Connection", "close");
    /**
     * simulate io to build the page
     */
    usleep(10000);
    Writer_start(wrtr, HTTP_STATUS_OK, resp_hdrs);
    Writer_write_chunk(wrtr, (void*) body, body_len);

    return_value = 1;

    finalize:
    if(resp_hdrs != NULL) HdrList_free(&resp_hdrs);
    if(body != NULL) free(body);
    if(body_len_str != NULL) free(body_len_str);
    return return_value;
}
 int handler_dispatch(MessageRef request, WriterRef wrtr)
 {
 }