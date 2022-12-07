#ifndef mytype_h
#define mytype_h

typedef struct MyType_s {
    int a;
} MyType, *MyTypeRef;

MyTypeRef MyType_new();
void MyType_free(MyTypeRef this);
void MyType_dealloc(void** this_ptr);
void MyType_dispose(MyType** ptr);
void MyType_display(MyTypeRef this);
#endif