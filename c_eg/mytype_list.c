#include <c_eg/mytype.h>
#include <c_eg/mytype_list.h>
#include <c_eg/alloc.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dealloc(void* ptr) {MyType_free((MTRef) ptr);}

MTList*  MTList_new() {return (MTList*)List_new(dealloc);}
void         MTList_free(MTList* lref) {List_free(lref);}
int          MTList_size(MTList* lref) {return List_size(lref);}
MTRef  MTList_first(MTList* lref) { return (MyTypeRef)List_first(lref);}
MTRef  MTList_last(MTList* lref)  { return (MyTypeRef)List_last(lref);}
MTRef  MTList_remove_first(MTList* lref) { return (MyTypeRef)List_remove_first(lref);}
MTRef  MTList_remove_last(MTList* lref) { return (MyTypeRef)List_remove_last(lref);}
MTRef  MTList_itr_unpack(MTList* lref, MTListIter iter) { return (MyTypeRef)List_itr_unpack(lref, iter);}
MTListIter MTList_iterator(MTList* lref) { return List_iterator(lref);}
MTListIter MTList_itr_next(MTList* lref, MTListIter iter) { return List_itr_next(lref, iter);}

void MTList_add_back(MTList* lref, MTRef item) {List_add_back(lref, (void*)item);}
void MTList_add_front(MTList* lref, MTRef item) {List_add_front(lref, (void*)item);}
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
