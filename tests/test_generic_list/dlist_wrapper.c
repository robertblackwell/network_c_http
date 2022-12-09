#include "dlist.h"


static void dealloc(void **ptr)
{
    TYPED(_dispose)(ptr);
}

PREFIX(Ref) PREFIX(_new)()
{
    return (PREFIX(Ref)) DList_new(dealloc);
}

void PREFIX(_dispose)(PREFIX(Ref) *lref_ptr)
{
    DList_dispose(lref_ptr);
}

int PREFIX(_size)(PREFIX(Ref) lref)
{
    return DList_size(lref);
}

TYPE* PREFIX(_first)(PREFIX(Ref) lref)
{
    return (TYPE*) DList_first(lref);
}

TYPE* PREFIX(_last)(PREFIX(Ref) lref)
{
    return (TYPE*) DList_last(lref);
}

TYPE* PREFIX(_remove_first)(PREFIX(Ref) lref)
{
    return (TYPE*) DList_remove_first(lref);
}

TYPE* PREFIX(_remove_last)(PREFIX(Ref) lref)
{
    return (TYPE*) DList_remove_last(lref);
}

TYPE* PREFIX(_itr_unpack)(PREFIX(Ref) lref, PREFIX(Iter) iter)
{
    return (TYPE*) DList_itr_unpack(lref, iter);
}

PREFIX(Iter) PREFIX(_iterator)(PREFIX(Ref) lref)
{
    return DList_iterator(lref);
}

PREFIX(Iter) PREFIX(_itr_next)(PREFIX(Ref) lref, PREFIX(Iter) iter)
{
    return DList_itr_next(lref, iter);
}

void PREFIX(_itr_remove)(PREFIX(Ref) lref, PREFIX(Iter) *iter)
{
    DList_itr_remove(lref, iter);
}

void PREFIX(_add_back)(PREFIX(Ref) lref, TYPE* item)
{
    PREFIX(Ref) tmp = lref;
    DList_add_back(lref, (void *) item);
}

void PREFIX(_add_front)(PREFIX(Ref) lref, TYPE* item)
{
    DList_add_front(lref, (void *) item);
}
void PREFIX(_display)(const PREFIX(Ref) this)
{
    DList_display(this);
}
