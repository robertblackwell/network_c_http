#ifndef c_http_sync_handler_example_h
#define c_http_sync_handler_example_h

#include <src/http/http_message.h>
#include <src/sync/sync.h>
HttpMessageRef app_handler_example(HttpMessageRef request, sync_worker_r wref);

#endif