
static void dealloc(void** ptr) {KVPair_dispose((KVPairRef*) ptr);}

HdrListRef  HdrList_new() {return (HdrListRef)List_new(dealloc);}
void         HdrList_dispose(HdrListRef* lref_ptr) {List_dispose(lref_ptr);}
int          HdrList_size(HdrListRef lref) {return List_size(lref);}
KVPairRef  HdrList_first(HdrListRef lref) { return (KVPairRef)List_first(lref);}
KVPairRef  HdrList_last(HdrListRef lref)  { return (KVPairRef)List_last(lref);}
KVPairRef  HdrList_remove_first(HdrListRef lref) { return (KVPairRef)List_remove_first(lref);}
KVPairRef  HdrList_remove_last(HdrListRef lref) { return (KVPairRef)List_remove_last(lref);}
KVPairRef  HdrList_itr_unpack(HdrListRef lref, HdrListIter iter) { return (KVPairRef)List_itr_unpack(lref, iter);}
HdrListIter HdrList_iterator(HdrListRef lref) { return List_iterator(lref);}
HdrListIter HdrList_itr_next(HdrListRef lref, HdrListIter iter) { return List_itr_next(lref, iter);}
void               HdrList_itr_remove(HdrListRef lref, HdrListIter* iter) { List_itr_remove(lref, iter);}

void HdrList_add_back(HdrListRef lref, KVPairRef item) {List_add_back(lref, (void*)item);}
void HdrList_add_front(HdrListRef lref, KVPairRef item) {List_add_front(lref, (void*)item);}
