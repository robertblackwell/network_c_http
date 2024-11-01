#ifndef rbl_macros_h
#define rbl_macros_h

#include <stdio.h>
#include <rbl/logger.h>
/**
 * @addtogroup group_macros
 * @{
 */

#define RBL_ASSERT(test, msg) \
do { \
    if(!test) { \
        RBL_LOG_ERROR("RBL_ASSERT failed file: %s line %d msg: %s", __FILE__, __LINE__, msg ); \
        assert(test); \
    } \
} while(0)

#define RBL_FATAL_ERROR(msg) \
do { \
    RBL_LOG_ERROR("Fatal error file: %s line %d msg: %s", __file__, __line__, msg ); \
    assert(false); \
} while(0)

/** @} */
#endif