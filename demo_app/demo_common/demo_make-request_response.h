#ifndef C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
#define C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
#include <http_in_c/demo_protocol/demo_message.h>

DemoMessageRef demo_make_request();
void demo_process_request(void* handler, DemoMessageRef request, DemoMessageRef reply);
bool demo_verify_response(DemoMessageRef request, DemoMessageRef response);

#endif //C_HTTP_Demo_MAKE_REQUEST_RESPONSE_H
