#include <c_eg/mytype.h>
#include <stdlib.h>
#include <c_eg/alloc.h>

#include <c_eg/utils.h>
struct MyType_s {
    int value_1;
    int value_2;
};

MyTypeRef MyType_new()
{
    MyTypeRef mref = eg_alloc(sizeof(MyType));
    mref->value_1 = 0;
    mref->value_2 = 0;
    return  mref;
}
void MyType_free(MyTypeRef* mref)
{
    ASSERT_NOT_NULL(*mref);
    free((void*) *mref);
    *mref = NULL;
}
