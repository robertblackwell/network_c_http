#ifndef c_ceg_oprlist_h
#define c_ceg_oprlist_h
#include <c_http/operation.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <c_http/dsl/list.h>
typedef List OprList;

typedef OprList* OprListRef;
typedef ListIter OprListIter;


#define M_OprList_new() List_new(dealloc)
#define M_OprList_dispose(lref) List_dispose(lref)
#define M_OprList_first(lref) (OperationListRef)List_first(lref)
#define M_OprList_size(lref) (OperationListRef)List_size(lref)
#define M_OprList_last(lref) (OperationListRef)List_last(lref)
#define M_OprList_remove_first(lref) (OperationListRef)List_remove_first(lref)
#define M_OprList_remove_last(lref) (OperationListRef)List_remove_last(lref)
#define M_OprList_itr_unpack(lref, iter) (OperationListRef)List_itr_unpack(lref, iter)
#define M_OprList_iterator(lref) List_iterator(lref)
#define M_OprList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_OprList_itr_remove(lref, itr)

#define M_OprList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_OprList_add_front(lref, item) List_add_back(lref, (void*)item);


OprListRef  OprList_new();
void OprList_dispose(OprListRef* lref_ptr) ;
int  OprList_size(OprListRef lref);

Operation*  OprList_first(OprListRef lref);
Operation*  OprList_last(OprListRef lref) ;
Operation*  OprList_remove_first(OprListRef lref);
Operation*  OprList_remove_last(OprListRef lref);
Operation*  OprList_itr_unpack(OprListRef lref, OprListIter iter);
OprListIter OprList_iterator(OprListRef lref);
OprListIter OprList_itr_next  (OprListRef lref, OprListIter iter);
void               OprList_itr_remove(OprListRef lref, OprListIter* iter);

void OprList_add_back(OprListRef lref, Operation* item);
void OprList_add_front(OprListRef lref, Operation* item);
#endif
