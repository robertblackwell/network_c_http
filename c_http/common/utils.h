#ifndef c_http_utils_h
#define c_http_utils_h
#include <stdlib.h>
#include <assert.h>

/**
 * @addtogroup group_util
 * @{
 */

#define ASSERT_NOT_NULL(ptr) assert(ptr != NULL);

char* make_upper(const char* src);

/** @} */
#endif