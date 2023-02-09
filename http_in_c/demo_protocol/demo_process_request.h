#ifndef C_HTTP_DEMO_PROCESS_REQUEST_H
#define C_HTTP_DEMO_PROCESS_REQUEST_H
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_handler.h>

DemoMessageRef process_request(DemoHandlerRef href, DemoMessageRef request);

#endif