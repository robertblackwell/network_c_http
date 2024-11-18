#ifndef C_HTTP_DEMO_PROCESS_REQUEST_H
#define C_HTTP_DEMO_PROCESS_REQUEST_H
#include <http_in_c/http/http_message.h>
#include <http_in_c/http_protocol/http_handler.h>

HttpMessageRef process_request(HttpHandlerRef href, HttpMessageRef request);

#endif