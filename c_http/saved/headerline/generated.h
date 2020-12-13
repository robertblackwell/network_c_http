#include <c_http/dsl/list.h>
typedef ListRef HdrListRef;
typedef ListNode* HdrListIter, ListIter;


#define M_HdrList_new() List_new(dealloc)
#define M_HdrList_free(lref) List_free(lref)
#define M_HdrList_first(lref) (KVPairListRef)List_first(lref)
#define M_HdrList_size(lref) (KVPairListRef)List_size(lref)
#define M_HdrList_last(lref) (KVPairListRef)List_last(lref)
#define M_HdrList_remove_first(lref) (KVPairListRef)List_remove_first(lref)
#define M_HdrList_remove_last(lref) (KVPairListRef)List_remove_last(lref)
#define M_HdrList_itr_unpack(lref, iter) (KVPairListRef)List_itr_unpack(lref, iter)
#define M_HdrList_iterator(lref) List_iterator(lref)
#define M_HdrList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_HdrList_itr_remove(lref, itr)

#define M_HdrList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_HdrList_add_front(lref, item) List_add_back(lref, (void*)item);


HdrListRef  HdrList_new();
void HdrList_free(HdrListRef* lref_ptr) ;
int  HdrList_size(HdrListRef lref);

KVPairRef  HdrList_first(HdrListRef lref);
KVPairRef  HdrList_last(HdrListRef lref) ;
KVPairRef  HdrList_remove_first(HdrListRef lref);
KVPairRef  HdrList_remove_last(HdrListRef lref);
KVPairRef  HdrList_itr_unpack(HdrListRef lref, HdrListIter iter);
HdrListIter HdrList_iterator(HdrListRef lref);
HdrListIter HdrList_itr_next  (HdrListRef lref, HdrListIter iter);
void               HdrList_itr_remove(HdrListRef lref, HdrListIter* iter);

void HdrList_add_back(HdrListRef lref, KVPairRef item);
void HdrList_add_front(HdrListRef lref, KVPairRef item);

