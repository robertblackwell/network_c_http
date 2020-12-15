#ifndef c_http_test_helpers_reader_private_h
#define c_http_test_helpers_reader_private_h
/**
 * @addtogroup group_test_helpers Reader_private
 * @brief A private interface to Reader module strictly for testing internals of the reader
 * @{
 */

#include <c_http/details/rdsocket.h>
#include <c_http/api/message.h>

ReaderRef Reader_private_new(RdSocket rdsock);
void Reader_private__init(ReaderRef this, RdSocket rdsock);
/** @} */
#endif