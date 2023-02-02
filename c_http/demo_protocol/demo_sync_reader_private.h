#ifndef c_demo_test_helpers_reader_private_h
#define c_demo_test_helpers_reader_private_h

#include <c_http/http_parser/rdsocket.h>
#include <c_http/demo_protocol/demo_message.h>

DemoSyncReaderRef demosync_reader_private_new(RdSocket rdsock);
void demosync_reader_private__init(DemoSyncReaderRef this, RdSocket rdsock);
/** @} */
#endif