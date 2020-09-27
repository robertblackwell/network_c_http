#include <c_eg/list.h>
typedef List* HdrList*;
typedef ListNode* HdrListIter, ListIter;


#define M_HdrList_new() List_new(dealloc)
#define M_HdrList_free(lref) List_free(lref)
#define M_HdrList_first(lref) (KVPairList*)List_first(lref)
#define M_HdrList_size(lref) (KVPairList*)List_size(lref)
#define M_HdrList_last(lref) (KVPairList*)List_last(lref)
#define M_HdrList_remove_first(lref) (KVPairList*)List_remove_first(lref)
#define M_HdrList_remove_last(lref) (KVPairList*)List_remove_last(lref)
#define M_HdrList_itr_unpack(lref, iter) (KVPairList*)List_itr_unpack(lref, iter)
#define M_HdrList_iterator(lref) List_iterator(lref)
#define M_HdrList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_HdrList_itr_remove(lref, itr)

#define M_HdrList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_HdrList_add_front(lref, item) List_add_back(lref, (void*)item);


HdrList*  HdrList_new();
void HdrList_free(HdrList** lref_ptr) ;
int  HdrList_size(HdrList* lref);

KVPair*  HdrList_first(HdrList* lref);
KVPair*  HdrList_last(HdrList* lref) ;
KVPair*  HdrList_remove_first(HdrList* lref);
KVPair*  HdrList_remove_last(HdrList* lref);
KVPair*  HdrList_itr_unpack(HdrList* lref, HdrListIter iter);
HdrListIter HdrList_iterator(HdrList* lref);
HdrListIter HdrList_itr_next  (HdrList* lref, HdrListIter iter);
void               HdrList_itr_remove(HdrList* lref, HdrListIter* iter);

void HdrList_add_back(HdrList* lref, KVPair* item);
void HdrList_add_front(HdrList* lref, KVPair* item);

