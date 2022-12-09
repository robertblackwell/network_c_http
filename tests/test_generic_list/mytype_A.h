#ifndef _mytype_a_h
#define _mytype_a_h

typedef struct MyTypeA_s {
    int a;
} MyTypeA, *MyTypeARef;

MyTypeARef MyTypeA_new();
void MyTypeA_free(MyTypeARef this);
void MyTypeA_dealloc(void** this_ptr);
void MyTypeA_dispose(MyTypeA** ptr);
void MyTypeA_display(MyTypeARef this);
#endif