
#include <src/sync/sync_handler_example.h>
#include <src/sync/sync.h>
#include <src/common/alloc.h>
#include <src/common/socket_functions.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

static BufferChainRef simple_response_body(char* message, socket_handle_t socket, int pthread_self_value)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));
    BufferChainRef bchain = BufferChain_new();
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
    BufferChain_append_cstr(bchain, s1);
    free(s1);
    return bchain;
}
/**
 * Create the body of a response to an /echo request
 * \param request
 * \return char* ownership, and responsibility to free, transfers to the caller
 */
static char* echo_body(HttpMessageRef request)
{
    IOBufferRef iob_body = http_message_serialize(request);
    char* body = malloc(IOBuffer_data_len(iob_body) + 1);
    memcpy(body, IOBuffer_data(iob_body), IOBuffer_data_len(iob_body)+1);
    return body;
}

static BufferChainRef echo_body_buffer_chain(HttpMessageRef request)
{
    IOBufferRef iob_body = http_message_serialize(request);
    BufferChainRef bufchain = BufferChain_new();
    BufferChain_append_IOBuffer(bufchain, iob_body);
    return bufchain;
}
HttpMessageRef app_handler_example(HttpMessageRef request, sync_worker_r wref)
{
    HttpMessageRef response = http_message_new_response();
    char* msg = "<h2>this is a message</h2>";
    BufferChainRef body_chain = NULL;
    char* body = NULL;
    char* body_len_str = NULL;
    int return_value = 0;

    CbufferRef target = http_message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        /**
         * find the echo-id header in the request and put it in the response headers
         */
        const char* echo_value = http_message_get_header_value(request, HEADER_ECHO_ID);
        if(echo_value != NULL) {
            http_message_add_header_cstring(response, HEADER_ECHO_ID, echo_value);
        }
        body_chain = echo_body_buffer_chain(request);
    } else {

        body_chain = simple_response_body(msg, sync_worker_socketfd(wref), sync_worker_pthread(wref));
    }
    http_message_set_status(response, HTTP_STATUS_OK);
    http_message_set_reason(response, "OK");
    http_message_add_header_cstring(response, HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");

//    int cmp_tmp = HttpMessage_cmp_header(request, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
//    if(cmp_tmp == 1) {
//        http_message_add_header_cstring(response, HEADER_CONNECTION_KEY, HEADER_CONNECTION_KEEPALIVE);
//    } else {
//        http_message_add_header_cstring(response, HEADER_CONNECTION_KEY, HEADER_CONNECTION_CLOSE);
//    }

//    IOBufferRef request_serialized = http_message_serialize(request);
//    IOBufferRef response_serialized = http_message_serialize(response);
//    printf("app_handler_example request : %s\n", IOBuffer_cstr(request_serialized));
//    printf("app_handler_example response : %s\n", IOBuffer_cstr(response_serialized));
//    IOBuffer_free(request_serialized);
//    IOBuffer_free(response_serialized);
    http_message_set_body(response, body_chain);
    http_message_set_content_length(response, BufferChain_size(body_chain));
    return response;
}
