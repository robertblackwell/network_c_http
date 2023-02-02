#ifndef c_http_sync_writer_h
#define c_http_sync_writer_h
#include <c_http/common/message.h>
/**
 * @addtogroup group_writer Writer
 * @brief A module that implements a synchronous writer of http/1.1 messages
 * @{
 */


typedef struct SyncWriter_s Writer, *SyncWriterRef;

void SyncWriter_init(SyncWriterRef this, int sock);
SyncWriterRef SyncWriter_new(int sock);
void SyncWriter_destroy(SyncWriterRef this);
void SyncWriter_dispose(SyncWriterRef* this_ptr);

void SyncWriter_write(SyncWriterRef wrtr, MessageRef msg_ref);
//void SyncWriter_start(SyncWriterRef this, HttpStatus status, HdrListRef headers);
void SyncWriter_write_chunk(SyncWriterRef this, void* buffer, int len);
int SyncWriter_sock_fd(SyncWriterRef this);

/** @}*/
#endif