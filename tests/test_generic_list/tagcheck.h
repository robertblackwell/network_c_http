#ifndef _tagcheck_h
#define _tagcheck_h
#include <stdbool.h>
#include <string.h>


#ifdef TYPE_CHECK_ON
#define TAG_LENGTH 10
#define DECLARE_TAG(TAG) char tag[TAG_LENGTH]
#define CHECK_TAG_FIELD(TAG, p, field) \
    do { \
        if(strcmp((p)->field, TAG) != 0) { \
            assert(false);                  \
        } \
    } while(0);

#define CHECK_TAG(TAG, p) \
    do { \
        if(strcmp((p)->tag, TAG) != 0) { \
            assert(false);                  \
        } \
    } while(0);

// used for testing only
#define FAIL_CHECK_TAG(TAG, p) \
    do { \
        assert(strcmp((p)->tag, TAG) != 0); \
    } while(0);

#define SET_TAG_FIELD(TAG, p, field) \
    do {                     \
        static_assert(strlen(TAG) < TAG_LENGTH, "Tag too long in SET_TAG");                     \
        sprintf((p)->field, "%s", TAG); \
    } while(0);


#define SET_TAG(TAG, p) \
    do {                     \
        static_assert(strlen(TAG) < TAG_LENGTH, "Tag too long in SET_TAG");                     \
        sprintf((p)->tag, "%s", TAG); \
    } while(0);
#else
    #define DECLARE_TAG(TYPE)
    #define CHECK_TAG(TYPE, p)
    #define SET_TAG(TYPE, p)
#endif


#endif