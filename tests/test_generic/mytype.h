#ifndef mytype_h
#define mytype_h

typedef struct MyType_s {
    int a;
} MyType, *MyTypeRef;

MyTypeRef MyType_new();
void MyType_free(MyTypeRef this);

#define TYPE MyType
#define PREFIX MyType
#include "array_template.h"
#undef TYPE
#undef PREFIX

#endif