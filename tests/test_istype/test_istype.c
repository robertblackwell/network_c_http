#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct SomeClass_s {
    int prop1;
} SomeClass, *SomeClassRef;

void SomeClass_free(SomeClassRef* this_ptr){}

#define isType(x, C) _Generic((x), C: 1, default: 0)
#define CheckIsType(x, C) _Static_assert(isType(x, C) == 1, "Not of required class ")

#define isSomeClassRef(x) _Generic((x),  SomeClassRef*: 1, default:0)
#define CheckIsRefStar(x) _Static_assert(isSomeClassRef(x) == 1, "Not a SomeClassRef")

#define SomeClass_FREE2(x) \
    CheckIsType(x, SomeClassRef*); \
    SomeClass_free(x);

#define SomeClass_FREE(x) \
    CheckIsRefStar(x); \
    SomeClass_free(x);

#define MyMacro(a, b) do { \
        int i = 2; \
        int j = 4; \
        printf("inside macro a: %d b: %d i: %d j: %d \n",a,b, i, j); \
    } while(0);

int main()
{
    SomeClass* someclassptr;
    SomeClassRef someref;
    int a;
    int b;
    MyMacro(a,b)
}