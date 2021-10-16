
/**
 * @addtogroup group_check_tags
 * These macros add identity tags to a struct, set the value of that identity tag and test the value
 * I find these macros usefull in situations using c callbacks where there is a log of casting of void* to a typed pointer
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
 *      DECLARE_TAG(YourType)
 *      ...
 *      ...
 * }
 *
 * Then at strategic points in your code
 *
 * void callback(void* arg) {
 *      YourType* y = (YourType*)arg;
 *      CHECK_YOURTYPE(y)
 * @{
 */

#ifdef TYPE_CHECK_ON
#define TAG_LENGTH 8
    #define TAG(TYPE) TYPE ## _TAG
    #define DECLARE_TAG(TYPE) char tag[TAG_LENGTH]
    #define CHECK_TAG(TYPE, p) \
    do { \
        assert(strcmp((p)->tag, TAG(TYPE)) == 0); \
    } while(0);

    #define SET_TAG(TYPE, p) \
    do { \
        sprintf((p)->tag, "%s", TAG(TYPE)); \
    } while(0);
#else
#define DECLARE_TAG(TYPE)
#define CHECK_TAG(TYPE, p)
#define SET_TAG(TYPE, p)

#endif
/** @} */
