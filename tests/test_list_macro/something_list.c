typedef struct MyType_s {
    int a;
    int b;
} MyType, *MyTypeRef;

#define T MyTypeRef
#include "vector.h";