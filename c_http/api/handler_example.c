#define _GNU_SOURCE
#include <c_http/api/handler_example.h>

#include <c_http/details/worker.h>
#include <c_http/constants.h>
#include <c_http/dsl/alloc.h>
#include <c_http/dsl/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/dsl/queue.h>
#include <c_http/details/ll_parser.h>
#include <c_http/api/reader.h>
#include <c_http/api/writer.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <pthread.h>


BufferChainRef simple_response_body(char* message, socket_handle_t socket, int pthread_self_value)
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
char* echo_body(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    char* body = malloc(IOBuffer_data_len(iob_body) + 1);
    memcpy(body, IOBuffer_data(iob_body), IOBuffer_data_len(iob_body)+1);
    return body;
}
BufferChainRef echo_body_buffer_chain(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    BufferChainRef bufchain = BufferChain_new();
    BufferChain_append_IOBuffer(bufchain, iob_body);
    return bufchain;
}
int handler_example(MessageRef request, WriterRef wrtr)
{
    MessageRef response = Message_new_response();
    char* msg = "<h2>this is a message</h2>";
    BufferChainRef body_chain = NULL;
    char* body = NULL;
    char* body_len_str = NULL;
    int return_value = 0;

    CbufferRef target = Message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        /**
         * find the echo-id header in the request and put it in the response headers
         */
        const char* echo_value = Message_get_header_value(request, HEADER_ECHO_ID);
        if(echo_value != NULL) {
            Message_add_header_cstring(response, HEADER_ECHO_ID, echo_value);
        }
        body_chain = echo_body_buffer_chain(request);
    } else {
        body_chain = simple_response_body(msg, Writer_sock_fd(wrtr), pthread_self());
    }

    Message_add_header_cstring(response, HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");
    Message_add_header_cstring(response, "Connection", "close");
    /**
     * simulate io to build the page
     */
//    usleep(1000);
    Message_set_body(response, body_chain);
    Message_set_content_length(response, BufferChain_size(body_chain));
    Writer_write(wrtr, response);

    return_value = 1;

    finalize:
    if(body != NULL) free(body);
    if(body_len_str != NULL) free(body_len_str);
    return return_value;
}
 int handler_dispatch(MessageRef request, WriterRef wrtr)
 {
 }