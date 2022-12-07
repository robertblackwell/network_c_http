#include "mytype.h"
#include "mytype_list.h"
#include "mytype_wrapped_list.h"
int main()
{
    MyTypeList *list = MyTypeList_new();
    MyType* mtp = MyType_new();
    mtp->a = 32;
    MyTypeList_add_back(list, mtp);
    MyTypeList_display(list);
    MyTypeList_dispose(&list);
}