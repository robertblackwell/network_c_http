#ifndef c_demo_sync_writer_h
#define c_demo_sync_writer_h
#include <http_in_c/demo_protocol/demo_message.h>


typedef struct DemoSyncWriter_s DemoSyncWriter, *DemoSyncWriterRef;

void demosync_writer_init(DemoSyncWriterRef this, int sock);
DemoSyncWriterRef demosync_writer_new(int sock);
void demosync_writer_destroy(DemoSyncWriterRef this);
void demosync_writer_dispose(DemoSyncWriterRef* this_ptr);

void demosync_writer_write(DemoSyncWriterRef wrtr, DemoMessageRef msg_ref);
void demosync_writer_write_chunk(DemoSyncWriterRef this, void* buffer, int len);
int demosync_writer_sock_fd(DemoSyncWriterRef this);

/** @}*/
#endif