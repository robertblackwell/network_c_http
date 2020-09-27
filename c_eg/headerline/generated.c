
static void dealloc(void** ptr) {KVPair_free((KVPair**) ptr);}

HdrList*  HdrList_new() {return (HdrList*)List_new(dealloc);}
void         HdrList_free(HdrList** lref_ptr) {List_free(lref_ptr);}
int          HdrList_size(HdrList* lref) {return List_size(lref);}
KVPair*  HdrList_first(HdrList* lref) { return (KVPair*)List_first(lref);}
KVPair*  HdrList_last(HdrList* lref)  { return (KVPair*)List_last(lref);}
KVPair*  HdrList_remove_first(HdrList* lref) { return (KVPair*)List_remove_first(lref);}
KVPair*  HdrList_remove_last(HdrList* lref) { return (KVPair*)List_remove_last(lref);}
KVPair*  HdrList_itr_unpack(HdrList* lref, HdrListIter iter) { return (KVPair*)List_itr_unpack(lref, iter);}
HdrListIter HdrList_iterator(HdrList* lref) { return List_iterator(lref);}
HdrListIter HdrList_itr_next(HdrList* lref, HdrListIter iter) { return List_itr_next(lref, iter);}
void               HdrList_itr_remove(HdrList* lref, HdrListIter* iter) { List_itr_remove(lref, iter);}

void HdrList_add_back(HdrList* lref, KVPair* item) {List_add_back(lref, (void*)item);}
void HdrList_add_front(HdrList* lref, KVPair* item) {List_add_front(lref, (void*)item);}
