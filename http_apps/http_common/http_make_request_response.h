#ifndef C_HTTP_Http_REQUEST_RESPONSE_H
#define C_HTTP_Http_REQUEST_RESPONSE_H
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/common/buffer_chain.h>
#include "http_apps/http_verify_app/verify_thread_context.h"

HttpMessageRef http_make_request(char* url, bool keep_alive_flag);

/**
 * Turns a request into a response. The request and response message objects are owned
 * by the caller and should not be free'd inside this function.
 *
 * @param handler   a void* pointer to the handler object
 * @param request   HttpMessageRef the request message
 * @param response  HttpMessageRef an empty message for the response to be filled in
 */
void http_process_request(void* handler, HttpMessageRef request, HttpMessageRef reply);

bool http_verify_response(HttpMessageRef request, HttpMessageRef response);

#endif //C_HTTP_VERIFY_MK_REQUEST_H
