#ifndef c_http/macros_h
#define c_http_macros_h
#include <c_http/logger.h>
/**
 * @addtogroup group_macros
 * @{
 */

#define CHTTP_ASSERT(test, msg) \
do { \
    if(!test) { \
        LOG_ERROR("XR_ASSERT failed file: %s line %d msg: %s", __FILE__, __LINE__, msg ); \
        assert(test); \
    } \
} while(0)

#define CHTTP_FATAL_ERROR(msg) \
do { \
    LOG_ERROR("Fatal error file: %s line %d msg: %s", __file__, __line__, msg ); \
    assert(false); \
} while(0)

/** @} */
#endif