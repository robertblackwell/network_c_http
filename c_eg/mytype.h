#ifndef c_eg_mytype_h_guard
#define c_eg_mytype_h_guard

struct MyType_s;

typedef struct MyType_s MyType, *MyTypeRef;

MyTypeRef MyType_new();
void MyType_free(MyTypeRef* mref);

#endif