#include <stdlib.h>
#include <stdio.h>
#include "mytype_B.h"

MyTypeBRef MyTypeB_new()
{
    MyTypeBRef this = malloc(sizeof(MyTypeB));
    return this;
}
void MyTypeB_free(MyTypeBRef this)
{
    free(this);
}
void MyTypeB_dealloc(void** this_ptr)
{
    MyTypeB_free(*this_ptr);
    *this_ptr = NULL;
}

void MyTypeB_display(MyTypeBRef this)
{
    printf("MyTypeB[%p] b: %f \n", this, this->b);
}
void MyTypeB_dispose(MyTypeB** ptr)
{
    MyTypeB_dealloc(ptr);
}
