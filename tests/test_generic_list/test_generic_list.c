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

    MyTypeWList *wlist = MyTypeWList_new();
    MyType* mtp2 = MyType_new();
    mtp2->a = 32;
    MyTypeWList_add_back(wlist, mtp);
    MyTypeWList_display(wlist);
    MyTypeWList_dispose(&wlist);

}