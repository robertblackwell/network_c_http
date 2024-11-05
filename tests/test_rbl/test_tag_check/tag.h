/**
 * This file defines a set of macros that add identity tags to a struct, set the value of that identity tag and test the value
 * I find these macros usefull in situations using c callbacks where there is a log of casting of void* to a typed pointer
 * It is very easy during development to get the type wrong. These macros catch such errors early.
 *
 * This checking can be truned off once a "stable" code base has been established
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
 *
 */

