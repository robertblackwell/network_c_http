
static void dealloc(void* ptr) {__TYPE___free((__TYPE__Ref) ptr);}

__TYPE__ListRef  __PREFIX__List_new() {return (__TYPE__ListRef)List_new(dealloc);}
void         __PREFIX__List_free(__TYPE__ListRef lref) {List_free(lref);}
__TYPE__Ref  __PREFIX__List_first(__TYPE__ListRef lref) { return (__TYPE__Ref)List_first(lref);}
__TYPE__Ref  __PREFIX__List_last(DObjListRef lref)  { return (__TYPE__Ref)List_last(lref);}
__TYPE__Ref  __PREFIX__List_remove_first(DObjListRef lref) { return (__TYPE__Ref)List_remove_first(lref);}
__TYPE__Ref  __PREFIX__List_remove_last(DObjListRef lref) { return (__TYPE__Ref)List_remove_last(lref);}
__TYPE__Ref  __PREFIX__List_itr_unpack(DObjListRef lref, DObjListIter iter) { return (__TYPE__Ref)List_itr_unpack(lref, iter);}
__TYPE__ListIter __PREFIX__List_iterator(DObjListRef lref) { return List_iterator(lref);}
__TYPE__ListIter __PREFIX__List_itr_next(DObjListRef lref, DObjListIter iter) { return List_itr_next(lref, iter);}

