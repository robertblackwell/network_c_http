#ifndef C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
#define C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
#include <msg/msg_selection_header.h>

MSG_REF demo_make_request();

/**
 * Turns a request into a response. The request and response message objects are owned
 * by the caller and should not be free'd inside this function.
 *
 * @param handler   a void* pointer to the handler object
 * @param request   HttpMessageRef the request message
 * @param response  HttpMessageRef an empty message for the response to be filled in
 */
void demo_process_request(void* handler, MSG_REF request, MSG_REF reply);

bool demo_verify_response(MSG_REF request, MSG_REF response);

#endif //C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
