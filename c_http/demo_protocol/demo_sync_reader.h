#ifndef demo_sync_reader_h
#define demo_sync_reader_h
#include <c_http/demo_protocol/demo_message.h>
typedef struct DemoSyncReader_s DemoSyncReader, *DemoSyncReaderRef;

DemoSyncReaderRef demosync_reader_new(int rdsock_fd);
//void demosync_reader_init(DemoSyncReaderRef this, RdSocket rdsock);
void demosync_reader_destroy(DemoSyncReaderRef this);
void demosync_reader_dispose(DemoSyncReaderRef* this_ptr);

typedef enum demosync_reader_ReturnCode {
        READER_OK = 0,             // A message was returned
        READER_PARSE_ERROR = -1,   // An error in the format of the message was detected.
        READER_IO_ERROR = -2,      // An IO error occurred.
} demosync_reader_ReturnCode;

demosync_reader_ReturnCode demosync_reader_read(DemoSyncReaderRef this, DemoMessageRef* msgref_ptr);
/** @} */
#endif