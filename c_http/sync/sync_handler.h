#ifndef c_http_sync_handler_h
#define c_http_sync_handler_h

#include <c_http/common/message.h>
#include <c_http/sync/sync_writer.h>

typedef MessageRef(*SyncAppMessageHandler(MessageRef request));


typedef int(*SyncHandlerFunction)(MessageRef request, SyncWriterRef wrtr);
/** @} */
#endif