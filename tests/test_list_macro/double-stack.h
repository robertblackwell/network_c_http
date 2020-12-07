#ifndef _DOUBLE_STACK_H_
#define _DOUBLE_STACK_H_
#include <stdlib.h>
/**
 * double-stack.h
 *   Stacks of doubles.
 *
 * <insert MIT license>
 */

#undef TYPE
#undef TYPED

#define TYPE double
#define TYPED(THING) d_##THING

#include "generic-stack.h"

#endif // _DOUBLE_STACK_H_
