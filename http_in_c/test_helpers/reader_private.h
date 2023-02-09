#ifndef c_http_test_helpers_reader_private_h
#define c_http_test_helpers_reader_private_h
/**
 * @addtogroup group_test_helpers SyncReader_private
 * @brief A private interface to Reader module strictly for testing internals of the reader
 * @{
 */

#include <http_in_c/common/http_parser/rdsocket.h>
#include <http_in_c/http/message.h>

SyncReaderRef SyncReader_private_new(RdSocket rdsock);
void SyncReader_private__init(SyncReaderRef this, RdSocket rdsock);
/** @} */
#endif