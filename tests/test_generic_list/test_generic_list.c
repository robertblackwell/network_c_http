#include <stdio.h>
#include "mytype_A.h"
#include "mytype_A_dlist.h"
#include "mytype_B.h"
#include "mytype_B_dlist.h"
#include "mytype_wrapped_dlist.h"
#include "mytype_circular_list.h"

int main()
{
    MyTypeAChain *alist = MyTypeAChain_new();
    MyTypeA* mtpa = MyTypeA_new();
    mtpa->a = 32;
    MyTypeAChain_add_back(alist, mtpa);
    MyTypeAChain_display(alist);
    MyTypeAChain_dispose(&alist);

    MyTypeBChain *blist = MyTypeBChain_new();
    MyTypeB* mtpb = MyTypeB_new();
    mtpb->b = 32.045;
    MyTypeBChain_add_back(blist, mtpb);
    MyTypeBChain_display(blist);
    MyTypeBChain_dispose(&blist);

    MyTypeAWList *wlist = MyTypeAWList_new();
    DListRef lr = (DListRef)wlist;
    MyTypeA* mtp2 = MyTypeA_new();
    mtp2->a = 32;
    MyTypeAWList_add_back(wlist, mtp2);
    MyTypeAWList_display(wlist);
    MyTypeAWList_dispose(&wlist);

    MyTypeA_Circular * clist = MyTypeA_Circular_new(32);
    MyTypeA* mtp3 = MyTypeA_new();
    mtp3->a = 42;
    MyTypeA mtp4 = {.a=56};
    MyTypeA_Circular_add(clist, *mtp3);
    MyTypeA_Circular_add(clist, mtp4);
    MyTypeA_Circular_display(clist);
    MyTypeA_dispose(&mtp3);
    MyTypeA_Circular_display(clist);
    MyTypeA_Circular_free(clist);
    printf("done \n");

}