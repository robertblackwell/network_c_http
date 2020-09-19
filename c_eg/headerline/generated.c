
static void dealloc(void* ptr) {HeaderLine_free((HeaderLineRef*) ptr);}

HDRListRef  HDRList_new() {return (HDRListRef)List_new(dealloc);}
void         HDRList_free(HDRListRef* lref_ptr) {List_free(lref_ptr);}
int          HDRList_size(HDRListRef lref) {return List_size(lref);}
HeaderLineRef  HDRList_first(HDRListRef lref) { return (HeaderLineRef)List_first(lref);}
HeaderLineRef  HDRList_last(HDRListRef lref)  { return (HeaderLineRef)List_last(lref);}
HeaderLineRef  HDRList_remove_first(HDRListRef lref) { return (HeaderLineRef)List_remove_first(lref);}
HeaderLineRef  HDRList_remove_last(HDRListRef lref) { return (HeaderLineRef)List_remove_last(lref);}
HeaderLineRef  HDRList_itr_unpack(HDRListRef lref, HDRListIter iter) { return (HeaderLineRef)List_itr_unpack(lref, iter);}
HDRListIter HDRList_iterator(HDRListRef lref) { return List_iterator(lref);}
HDRListIter HDRList_itr_next(HDRListRef lref, HDRListIter iter) { return List_itr_next(lref, iter);}
void               HDRList_itr_remove(HDRListRef lref, HDRListIter* iter) { List_itr_remove(lref, iter);}

void HDRList_add_back(HDRListRef lref, HeaderLineRef item) {List_add_back(lref, (void*)item);}
void HDRList_add_front(HDRListRef lref, HeaderLineRef item) {List_add_front(lref, (void*)item);}
