#include <http_in_c/common/list.h>
typedef ListRef __PREFIX__ListRef;
typedef ListNode* __PREFIX__ListIter, ListIter;


#define M___PREFIX__List_new() List_new(dealloc)
#define M___PREFIX__List_dispose(lref) List_dispose(lref)
#define M___PREFIX__List_first(lref) (__TYPE__ListRef)List_first(lref)
#define M___PREFIX__List_size(lref) (__TYPE__ListRef)List_size(lref)
#define M___PREFIX__List_last(lref) (__TYPE__ListRef)List_last(lref)
#define M___PREFIX__List_remove_first(lref) (__TYPE__ListRef)List_remove_first(lref)
#define M___PREFIX__List_remove_last(lref) (__TYPE__ListRef)List_remove_last(lref)
#define M___PREFIX__List_itr_unpack(lref, iter) (__TYPE__ListRef)List_itr_unpack(lref, iter)
#define M___PREFIX__List_iterator(lref) List_iterator(lref)
#define M___PREFIX__List_itr_next(lref, iter) List_itr_next(lref, iter)
#define M___PREFIX__List_itr_remove(lref, itr)

#define M___PREFIX__List_add_back(lref, item) List_add_back(lref, (void*)item);
#define M___PREFIX__List_add_front(lref, item) List_add_back(lref, (void*)item);


__PREFIX__ListRef  __PREFIX__List_new();
void __PREFIX__List_dispose(__PREFIX__ListRef* lref_ptr) ;
int  __PREFIX__List_size(__PREFIX__ListRef lref);

__TYPE__Ref  __PREFIX__List_first(__PREFIX__ListRef lref);
__TYPE__Ref  __PREFIX__List_last(__PREFIX__ListRef lref) ;
__TYPE__Ref  __PREFIX__List_remove_first(__PREFIX__ListRef lref);
__TYPE__Ref  __PREFIX__List_remove_last(__PREFIX__ListRef lref);
__TYPE__Ref  __PREFIX__List_itr_unpack(__PREFIX__ListRef lref, __PREFIX__ListIter iter);
__PREFIX__ListIter __PREFIX__List_iterator(__PREFIX__ListRef lref);
__PREFIX__ListIter __PREFIX__List_itr_next  (__PREFIX__ListRef lref, __PREFIX__ListIter iter);
void               __PREFIX__List_itr_remove(__PREFIX__ListRef lref, __PREFIX__ListIter* iter);

void __PREFIX__List_add_back(__PREFIX__ListRef lref, __TYPE__Ref item);
void __PREFIX__List_add_front(__PREFIX__ListRef lref, __TYPE__Ref item);

