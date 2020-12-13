
static void dealloc(void* ptr) {MyType_free((MTRef) ptr);}

MTListRef  MTList_new() {return (MTListRef)List_new(dealloc);}
void         MTList_free(MTListRef lref) {List_free(lref);}
int          MTList_size(MTListRef lref) {return List_size(lref);}
MTRef  MTList_first(MTListRef lref) { return (MyTypeRef)List_first(lref);}
MTRef  MTList_last(MTListRef lref)  { return (MyTypeRef)List_last(lref);}
MTRef  MTList_remove_first(MTListRef lref) { return (MyTypeRef)List_remove_first(lref);}
MTRef  MTList_remove_last(MTListRef lref) { return (MyTypeRef)List_remove_last(lref);}
MTRef  MTList_itr_unpack(MTListRef lref, MTListIter iter) { return (MyTypeRef)List_itr_unpack(lref, iter);}
MTListIter MTList_iterator(MTListRef lref) { return List_iterator(lref);}
MTListIter MTList_itr_next(MTListRef lref, MTListIter iter) { return List_itr_next(lref, iter);}

void MTList_add_back(MTListRef lref, MTRef item) {List_add_back(lref, (void*)item);}
void MTList_add_front(MTListRef lref, MTRef item) {List_add_front(lref, (void*)item);}
