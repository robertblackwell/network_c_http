#define RBL_LOG_ENABLED
#define RBL_LOG_ALLOW_GLOBAL
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <src/common/make_uuid.h>
#include "http_make_request_response.h"

static BufferChainRef make_reply_body_from_request(HttpMessageRef request);

/**
 * Create a request message with target = /echo, an Echo_id header, empty body
 * \param ctx
 * \return HttpMessageRef with ownership
 */
HttpMessageRef http_make_request(char* url, bool last_request_flag)
{
    url = "/echo";
    char uuid_buffer[100];
    char* uuid_ptr = &(uuid_buffer[0]);
    make_uuid(&uuid_ptr);

    HttpMessageRef request = http_message_new();
    http_message_set_is_request(request, true);
    http_message_set_method(request, HTTP_GET);
    http_message_set_target(request, url );
    char* content_length = "0";

    http_message_add_header_cstring(request, HEADER_HOST, "ahost");
    http_message_add_header_cstring(request, "User-agent", "x15:x15-soundtrip client");
    if(last_request_flag) {
        http_message_add_header_cstring(request, HEADER_CONNECTION_KEY, "Close");
    } else {
        http_message_add_header_cstring(request, HEADER_CONNECTION_KEY, "Keep-Alive");//"Close");
    }
    http_message_add_header_cstring(request, HEADER_CONTENT_LENGTH, content_length);
    http_message_add_header_cstring(request, HEADER_ECHO_ID, uuid_ptr);
    http_message_set_body(request, BufferChain_new());
    IOBufferRef ib = http_message_serialize(request);
    return request;
}

void http_process_request(HttpMessageRef request, HttpMessageRef reply)
{
    http_message_set_is_request(reply, false);
    http_message_set_status(reply, HTTP_STATUS_OK);
    http_message_set_reason(reply, "OK");
    http_message_set_version(reply, 1,1);

    BufferChainRef response_body = make_reply_body_from_request(request);
    int reply_body_len = BufferChain_size(response_body);

    char* buffer;
    int ll = asprintf(&buffer, "%d", reply_body_len);

    http_message_add_header_cstring(reply, "CONNECTION", "KEEP-ALIVE");
    http_message_add_header_cstring(reply, "CONTENT-LENGTH", buffer);
    free(buffer);

    http_message_set_body(reply, response_body);
    IOBufferRef serialized_reply = http_message_serialize(reply);
}

/**
 * Verify that the response is correct based on the ctx->uid and request values
 * \param ctx       thread_context_t*
 * \param request   HttpMessageRef
 * \param response  HttpMessageRef
 * \return bool
 */
bool http_verify_response(HttpMessageRef request, HttpMessageRef response)
{
    if(response == NULL) {
        printf("verify_response failed response is NULL\n");
        return false;
    }
    if(request == NULL) {
        printf("verify_response failed request is NULL\n");
        return false;
    }
    const char* expected_response_body = IOBuffer_cstr(BufferChain_compact(make_reply_body_from_request(request)));
    const char* actual_response_body = IOBuffer_cstr(BufferChain_compact(http_message_get_body(response)));
    if(strcmp(expected_response_body, actual_response_body) != 0) {
        printf("http_verify_failed \n");
        printf("expected: [%s]\n", expected_response_body);
        printf("got     : [%s]\n", actual_response_body);
        return false;
    }
    return true;
}
static BufferChainRef make_reply_body_from_request(HttpMessageRef request)
{
    IOBufferRef  serialized_request = http_message_serialize(request);
    const char* target_str = http_message_get_target(request);
    const char* host_str = http_message_get_header_value(request, "HOST");
    const char* user_agent_str = http_message_get_header_value(request, "USER-AGENT");
    const char* connection_str = http_message_get_header_value(request, "CONNECTION");
    const char* content_length_str = http_message_get_header_value(request, "CONTENT-LENGTH");
    long int content_length = -1;
    if(content_length_str != NULL) {
        const char* nptr = content_length_str;
        char*       endptr;
        long int cl = strtol(nptr, &endptr, 10);
        if((nptr != endptr) && (*endptr == (char)0)) {
            content_length = cl;
        }
    }
    const char* echo_id_str = http_message_get_header_value(request, "C-HTTP-ECHO-ID");

    char* reply_body_buffer;
    int reply_body_len = asprintf(&reply_body_buffer, "%s: %s: %s: %s: %s: %s", target_str,host_str, user_agent_str, connection_str, content_length_str, echo_id_str);
    BufferChainRef response_body = BufferChain_new();
    BufferChain_append_cstr(response_body, reply_body_buffer);
    free(reply_body_buffer);
    IOBuffer_free(serialized_request);

    return response_body;
}
