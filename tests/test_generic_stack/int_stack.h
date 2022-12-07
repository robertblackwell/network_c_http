#ifndef _INT_STACK_H_
#define _INT_STACK_H_

/**
 * double-stack.h
 *   Stacks of doubles.
 *
 * <insert MIT license>
 */

#undef TYPE
#undef TYPED
#define TYPE int
#define TYPED(THING) Int##THING
#include "generic-stack.h"

#endif // _INT_STACK_H_