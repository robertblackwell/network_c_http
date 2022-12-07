#include <stdio.h>
#include <stdlib.h>
#if 1
#include "double-stack.h"
#include "int_stack.h"
#else
#undef TYPE
#undef TYPED
#define TYPE double
#define TYPED(THING) Double##THING
#include "generic-stack.h"
#include "generic-stack.c"

#undef TYPE
#undef TYPED
#define TYPE int
#define TYPED(THING) Int##THING
#include "generic-stack.h"
#include "generic-stack.c"
#endif

int main() {
    DoubleStack* dstackref = DoubleStack_new();
    DoubleStack_push(dstackref, 1.2345);
    DoubleStack_push(dstackref, 9.8765);
    IntStack* intstackref = IntStack_new();
    IntStack_push(intstackref, 2);
    IntStack_push(intstackref, 3);
    printf("hello \n");
}