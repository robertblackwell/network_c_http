#ifndef _DOUBLE_STACK_H_
#define _DOUBLE_STACK_H_

/**
 * double-stack.h
 *   Stacks of doubles.
 *
 * <insert MIT license>
 */

#undef TYPE
#undef TYPED
#define TYPE double
#define TYPED(THING) Double##THING
#include "generic-stack.h"
//#include "generic-stack.c"

#endif // _DOUBLE_STACK_H_