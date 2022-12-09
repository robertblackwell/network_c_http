#include <stdlib.h>
#include <stdio.h>
#include "mytype_A.h"

MyTypeARef MyTypeA_new()
{
    MyTypeARef this = malloc(sizeof(MyTypeA));
    return this;
}
void MyTypeA_free(MyTypeARef this)
{
    free(this);
}
void MyTypeA_dealloc(void** this_ptr)
{
    MyTypeA_free(*this_ptr);
    *this_ptr = NULL;
}

void MyTypeA_display(MyTypeARef this)
{
    printf("MyType[%p] a: %d \n", this, this->a);
}
void MyTypeA_dispose(MyTypeA** ptr)
{
    MyTypeA_dealloc((void**)ptr);
}
