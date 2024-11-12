#ifndef c_http_sync_handler_h
#define c_http_sync_handler_h

#include <http_in_c/http/message.h>
#include <http_in_c/saved/sync_writer.h>

typedef MessageRef(*SyncAppMessageHandler)(MessageRef request_ptr, void* worker_ptr);
typedef void(*SyncConnectionServerMessageHandler)(MessageRef request_ptr, void* worker_ptr);

typedef int(*SyncHandlerFunction)(MessageRef request, SyncWriterRef wrtr);
/** @} */
#endif