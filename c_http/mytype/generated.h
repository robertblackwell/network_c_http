#include <c_http/list.h>
typedef ListRef MTListRef;
typedef ListNode* MTListIter, ListIter;


#define M_MTList_new() List_new(dealloc)
#define M_MTList_free(lref) List_free(lref)
#define M_MTList_first(lref) (MyTypeListRef)List_first(lref)
#define M_MTList_size(lref) (MyTypeListRef)List_size(lref)
#define M_MTList_last(lref) (MyTypeListRef)List_last(lref)
#define M_MTList_remove_first(lref) (MyTypeListRef)List_remove_first(lref)
#define M_MTList_remove_last(lref) (MyTypeListRef)List_remove_last(lref)
#define M_MTList_itr_unpack(lref, iter) (MyTypeListRef)List_itr_unpack(lref, iter)
#define M_MTList_iterator(lref) List_iterator(lref)
#define M_MTList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_MTList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_MTList_add_front(lref, item) List_add_back(lref, (void*)item);


MTListRef  MTList_new();
void MTList_free(MTListRef lref) ;
int  MTList_size(MTListRef lref);

MyTypeRef  MTList_first(MTListRef lref);
MyTypeRef  MTList_last(MTListRef lref) ;
MyTypeRef  MTList_remove_first(MTListRef lref);
MyTypeRef  MTList_remove_last(MTListRef lref);
MyTypeRef  MTList_itr_unpack(MTListRef lref, MTListIter iter);
MTListIter MTList_iterator(MTListRef lref);
MTListIter MTList_itr_next(MTListRef lref, MTListIter iter);

void MTList_add_back(MTListRef lref, MyTypeRef item);
void MTList_add_front(MTListRef lref, MyTypeRef item);

