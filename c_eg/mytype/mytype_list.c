#include <c_eg/mytype.h>

MyTypeRef MyType_new()
{
}

Found it
///
/// The remainder of this file is generated code and will be over written at the next build
///

static void dealloc(void* ptr) {MyType_free((MyTypeRef) ptr);}

MyTypeListRef  MTList_new() {return (MyTypeListRef)List_new(dealloc);}
void         MTList_free(MyTypeListRef lref) {List_free(lref);}
MyTypeRef  MTList_first(MyTypeListRef lref) { return (MyTypeRef)List_first(lref);}
MyTypeRef  MTList_last(DObjListRef lref)  { return (MyTypeRef)List_last(lref);}
MyTypeRef  MTList_remove_first(DObjListRef lref) { return (MyTypeRef)List_remove_first(lref);}
MyTypeRef  MTList_remove_last(DObjListRef lref) { return (MyTypeRef)List_remove_last(lref);}
MyTypeRef  MTList_itr_unpack(DObjListRef lref, DObjListIter iter) { return (MyTypeRef)List_itr_unpack(lref, iter);}
MyTypeListIter MTList_iterator(DObjListRef lref) { return List_iterator(lref);}
MyTypeListIter MTList_itr_next(DObjListRef lref, DObjListIter iter) { return List_itr_next(lref, iter);}

