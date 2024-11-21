#ifndef c_http_sync_handler_example_h
#define c_http_sync_handler_example_h

#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/sync/sync.h>
HttpMessageRef app_handler_example(HttpMessageRef request, sync_worker_r wref);

#endif