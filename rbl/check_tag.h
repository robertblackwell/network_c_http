#ifndef H_c_http_rbl_check_tags_H
#define H_c_http_rbl_check_tags_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>


#ifdef RBL_TAG_CHECK_ON
#define RBL_TAG_LENGTH 15

#define RBL_DECLARE_TAG_FIELD(field) union{ char field[RBL_TAG_LENGTH]; int64_t __dummy_field[2];};
#define RBL_DECLARE_TAG union {char tag[RBL_TAG_LENGTH]; int64_t __dummy_tag[2];};
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
#define RBL_CHECK_TAG_PTR(TAG, p) \
    do { \
        if(strcmp((p), TAG) != 0) { \
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

#define RBL_SET_TAG_PTR(TAG, p) \
    do {                     \
        static_assert(strlen(TAG) < RBL_TAG_LENGTH, "Tag too long in RBL_SET_TAG");                     \
        sprintf((p), "%s", TAG); \
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
    #define RBL_CHECK_TAG_PTR(TAG, p)
    #define RBL_CHECK_END_TAG(TYPE, p)
    #define RBL_SET_TAG(TYPE, p)
    #define RBL_SET_TAG_PTR(TYPE, p)
    #define RBL_SET_END_TAG(TYPE, p)
    #define RBL_FAIL_CHECK_TAG(TYPE, p)
    #define RBL_INVALIDATE_TAG(p)
    #define RBL_INVALIDATE_END_TAG(p)
    #define RBL_INVALIDATE_STRUCT(p, TYPE)
#endif

#if defined(RBL_CHECK_TAG_INLINE)
inline size_t rbl_tag_size()
{
    typedef struct DummyTagStruct_s
    {
        RBL_DECLARE_TAG;
        uint64_t after_tag;
    } DummyTagStruct;
    size_t n = offsetof(DummyTagStruct, after_tag);
    return n;
}

#else
size_t rbl_tag_size();
#endif

/** @} */
#endif