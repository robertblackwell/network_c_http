#ifndef c_http_sync_handler_h
#define c_http_sync_handler_h

#include <c_http/common/message.h>
#include <c_http/saved/sync_writer.h>

typedef MessageRef(*SyncAppMessageHandler)(MessageRef request_ptr, void* worker_ptr);
typedef void(*SyncConnectionServerMessageHandler)(MessageRef request_ptr, void* worker_ptr);

typedef int(*SyncHandlerFunction)(MessageRef request, SyncWriterRef wrtr);
/** @} */
#endif