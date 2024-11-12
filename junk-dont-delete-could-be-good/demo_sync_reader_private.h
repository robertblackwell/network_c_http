#ifndef c_demo_test_helpers_reader_private_h
#define c_demo_test_helpers_reader_private_h

#include <http_in_c/saved/rdsocket.h>
#include <http_in_c/demo_protocol/demo_message.h>

DemoSyncReaderRef demosync_reader_private_new(RdSocket rdsock);
void demosync_reader_private__init(DemoSyncReaderRef this, RdSocket rdsock);
/** @} */
#endif