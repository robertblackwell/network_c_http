#ifndef mytype_h
#define mytype_h

typedef struct MyType_s {
    int a;
} MyType, *MyTypeRef;

MyTypeRef MyType_new();
void MyType_free(MyTypeRef this);

#endif