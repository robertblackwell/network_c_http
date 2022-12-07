#include <stdlib.h>
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

#define TYPE MyType
#define PREFIX MyType
#include "array_template.c"
#undef TYPE
#undef PREFIX


