#ifndef c_http_sync_handler_example_h
#define c_http_sync_handler_example_h

#include <c_http/common/message.h>
#include <c_http/sync/sync.h>
MessageRef app_handler_example(MessageRef request, sync_worker_r wref);

#endif