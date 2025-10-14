#ifndef H_rbl_check_tags_H
#define H_rbl_check_tags_H
/**
 * These macros add identity tags to a struct, set the value of that identity tag and test the value
 *
 * I find these macros usefull in situations using c callbacks where there is a lot of casting of void*
 * to a typed pointer
 *
 * It is very easy during development to get the type wrong. These macros catch such errors early.
 *
 * This checking can be turned off once a "stable" code base has been established
 *
 * Use this file as follows
 * 1. at the head of your implementation or maybe header file if the struct is declared there include the following lines
 *
 * #define TYPE YourType
 * #define YourType_TAG "YOURT"
 * #include "tag.h"
 * #undef TYPE
 * #define CHECK_YOURTYPE(p) CHECK_TAG(YourType, p)
 *
 * in your struct include the following
 *
 * struct YoutYpe_s {
 *      RBL_DECLARE_TAG(YourType)
 *      ...
 *      ...
 * }
 *
 * Then at strategic points in your code
 *
 * void callback(void* arg) {
 *      YourType* y = (YourType*)arg;
 *      RBL_CHECK_YOURTYPE(y)
 * 
 * 
 * NOTE: if you make a tag too long - you will get a runtime error NOT a compile time error
 */
#include <string.h>

#ifdef RBL_TAG_CHECK_ON
#define RBL_TAG_LENGTH 16

#define RBL_DECLARE_TAG_FIELD(field) char field[RBL_TAG_LENGTH]
#define RBL_DECLARE_TAG char tag[RBL_TAG_LENGTH]
#define RBL_DECLARE_END_TAG  RBL_DECLARE_TAG_FIELD(end_tag)

#define RBL_TAG_VALID(TAG, p) ((bool)(strcmp((p)->tag, TAG) == 0))
#define RBL_END_TAG_VALID(TAG, p) ((bool)(strcmp((p)->end_tag, TAG) == 0))

#define RBL_CHECK_TAG_FIELD(TAG, p, field) \
    do { \
        if(strcmp((p)->field, TAG) != 0) { \
            assert(false);                  \
        } \
    } while(0);

#define RBL_CHECK_TAG(TAG, p) \
    do { \
        if(strcmp((p)->tag, TAG) != 0) { \
            assert(false);                  \
        } \
    } while(0);
#define RBL_CHECK_END_TAG(TAG, p) \
    RBL_CHECK_TAG_FIELD(TAG, p, end_tag)

// used for testing only
#define RBL_FAIL_CHECK_TAG(TAG, p) \
    do { \
        assert(strcmp((p)->tag, TAG) != 0); \
    } while(0);

#define RBL_SET_TAG_FIELD(TAG, p, field) \
    do {                     \
        static_assert(strlen(TAG) < RBL_TAG_LENGTH, "Tag too long in RBL_SET_TAG");                     \
        sprintf((p)->field, "%s", TAG); \
    } while(0);


#define RBL_SET_TAG(TAG, p) \
    do {                     \
        static_assert(strlen(TAG) < RBL_TAG_LENGTH, "Tag too long in RBL_SET_TAG");                     \
        sprintf((p)->tag, "%s", TAG); \
    } while(0);

#define RBL_SET_END_TAG(TAG, p) \
    RBL_SET_TAG_FIELD(TAG, p, end_tag)

#define RBL_INVALID_TAG "invalid"
#define RBL_INVALIDATE_TAG(p) \
    do {                     \
        static_assert(strlen(RBL_INVALID_TAG) < RBL_TAG_LENGTH, "Tag too long in RBL_SET_TAG");                     \
        sprintf((p)->tag, "%s", RBL_INVALID_TAG); \
    } while(0);
#define RBL_INVALIDATE_END_TAG(p) \
    do {                     \
        static_assert(strlen(RBL_INVALID_TAG) < RBL_TAG_LENGTH, "Tag too long in RBL_SET_TAG");                     \
        sprintf((p)->end_tag, "%s", RBL_INVALID_TAG); \
    } while(0);

#define RBL_INVALIDATE_STRUCT(p, TYPE) \
    memset((void*)p, 0x00, sizeof(TYPE))
#else
    #define RBL_TAG_LENGTH 0
    #define RBL_TAG_VALID(TYPE, p) (1 == 1)
    #define RBL_DECLARE_TAG
    #define RBL_DECLARE_END_TAG
    #define RBL_CHECK_TAG(TYPE, p)
    #define RBL_CHECK_END_TAG(TYPE, p)
    #define RBL_SET_TAG(TYPE, p)
    #define RBL_SET_END_TAG(TYPE, p)
    #define RBL_FAIL_CHECK_TAG(TYPE, p)
    #define RBL_INVALIDATE_TAG(p)
    #define RBL_INVALIDATE_END_TAG(p)
    #define RBL_INVALIDATE_STRUCT(p, TYPE)
#endif
/** @} */

#endif
