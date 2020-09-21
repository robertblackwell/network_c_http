
static void dealloc(void** ptr) {__TYPE___free((__TYPE__Ref*) ptr);}

__PREFIX__ListRef  __PREFIX__List_new() {return (__PREFIX__ListRef)List_new(dealloc);}
void         __PREFIX__List_free(__PREFIX__ListRef* lref_ptr) {List_free(lref_ptr);}
int          __PREFIX__List_size(__PREFIX__ListRef lref) {return List_size(lref);}
__TYPE__Ref  __PREFIX__List_first(__PREFIX__ListRef lref) { return (__TYPE__Ref)List_first(lref);}
__TYPE__Ref  __PREFIX__List_last(__PREFIX__ListRef lref)  { return (__TYPE__Ref)List_last(lref);}
__TYPE__Ref  __PREFIX__List_remove_first(__PREFIX__ListRef lref) { return (__TYPE__Ref)List_remove_first(lref);}
__TYPE__Ref  __PREFIX__List_remove_last(__PREFIX__ListRef lref) { return (__TYPE__Ref)List_remove_last(lref);}
__TYPE__Ref  __PREFIX__List_itr_unpack(__PREFIX__ListRef lref, __PREFIX__ListIter iter) { return (__TYPE__Ref)List_itr_unpack(lref, iter);}
__PREFIX__ListIter __PREFIX__List_iterator(__PREFIX__ListRef lref) { return List_iterator(lref);}
__PREFIX__ListIter __PREFIX__List_itr_next(__PREFIX__ListRef lref, __PREFIX__ListIter iter) { return List_itr_next(lref, iter);}
void               __PREFIX__List_itr_remove(__PREFIX__ListRef lref, __PREFIX__ListIter* iter) { List_itr_remove(lref, iter);}

void __PREFIX__List_add_back(__PREFIX__ListRef lref, __TYPE__Ref item) {List_add_back(lref, (void*)item);}
void __PREFIX__List_add_front(__PREFIX__ListRef lref, __TYPE__Ref item) {List_add_front(lref, (void*)item);}
