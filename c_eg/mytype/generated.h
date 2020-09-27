#include <c_eg/list.h>
typedef List* MTList*;
typedef ListNode* MTListIter, ListIter;


#define M_MTList_new() List_new(dealloc)
#define M_MTList_free(lref) List_free(lref)
#define M_MTList_first(lref) (MyTypeList*)List_first(lref)
#define M_MTList_size(lref) (MyTypeList*)List_size(lref)
#define M_MTList_last(lref) (MyTypeList*)List_last(lref)
#define M_MTList_remove_first(lref) (MyTypeList*)List_remove_first(lref)
#define M_MTList_remove_last(lref) (MyTypeList*)List_remove_last(lref)
#define M_MTList_itr_unpack(lref, iter) (MyTypeList*)List_itr_unpack(lref, iter)
#define M_MTList_iterator(lref) List_iterator(lref)
#define M_MTList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_MTList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_MTList_add_front(lref, item) List_add_back(lref, (void*)item);


MTList*  MTList_new();
void MTList_free(MTList* lref) ;
int  MTList_size(MTList* lref);

MyTypeRef  MTList_first(MTList* lref);
MyTypeRef  MTList_last(MTList* lref) ;
MyTypeRef  MTList_remove_first(MTList* lref);
MyTypeRef  MTList_remove_last(MTList* lref);
MyTypeRef  MTList_itr_unpack(MTList* lref, MTListIter iter);
MTListIter MTList_iterator(MTList* lref);
MTListIter MTList_itr_next(MTList* lref, MTListIter iter);

void MTList_add_back(MTList* lref, MyTypeRef item);
void MTList_add_front(MTList* lref, MyTypeRef item);

