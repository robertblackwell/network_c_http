#include <stdlib.h>
#include <stdio.h>
#include "mytype.h"

MyTypeRef MyType_new()
{
    MyTypeRef this = malloc(sizeof(MyType));
    return this;
}
void MyType_free(MyTypeRef this)
{
    free(this);
}
void MyType_dealloc(void** this_ptr)
{
    MyType_free(*this_ptr);
    *this_ptr = NULL;
}

void MyType_display(MyTypeRef this)
{
    printf("MyType[%p] a: %d \n", this, this->a);
}
void MyType_dispose(MyType** ptr)
{
    MyType_dealloc(ptr);
}
