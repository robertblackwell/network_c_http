#ifndef c_http_sync_handler_example_h
#define c_http_sync_handler_example_h

#include <http_in_c/http/message.h>
#include <http_in_c/sync/sync.h>
MessageRef app_handler_example(MessageRef request, sync_worker_r wref);

#endif