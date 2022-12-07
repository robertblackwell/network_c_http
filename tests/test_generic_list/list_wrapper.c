#include <c_http/common/utils.h>


static void dealloc(void **ptr)
{
    TYPED(_dispose)(ptr);
}

TYPED(ListRef) TYPED(List_new)()
{
    return (TYPED(ListRef)) List_new(dealloc);
}

void TYPED(List_dispose)(TYPED(ListRef) *lref_ptr)
{
    List_dispose(lref_ptr);
}

int TYPED(List_size)(TYPED(ListRef) lref)
{
    return List_size(lref);
}

TYPE* TYPED(List_first)(TYPED(ListRef) lref)
{
    return (TYPE*) List_first(lref);
}

TYPE* TYPED(List_last)(TYPED(ListRef) lref)
{
    return (TYPE*) List_last(lref);
}

TYPE* TYPED(List_remove_first)(TYPED(ListRef) lref)
{
    return (TYPE*) List_remove_first(lref);
}

TYPE* TYPED(List_remove_last)(TYPED(ListRef) lref)
{
    return (TYPE*) List_remove_last(lref);
}

TYPE* TYPED(List_itr_unpack)(TYPED(ListRef) lref, TYPED(ListIter) iter)
{
    return (TYPE*) List_itr_unpack(lref, iter);
}

TYPED(ListIter) TYPED(List_iterator)(TYPED(ListRef) lref)
{
    return List_iterator(lref);
}

TYPED(ListIter) TYPED(List_itr_next)(TYPED(ListRef) lref, TYPED(ListIter) iter)
{
    return List_itr_next(lref, iter);
}

void TYPED(List_itr_remove)(TYPED(ListRef) lref, TYPED(ListIter) *iter)
{
    List_itr_remove(lref, iter);
}

void TYPED(List_add_back)(TYPED(ListRef) lref, TYPE* item)
{
    List_add_back(lref, (void *) item);
}

void TYPED(List_add_front)(TYPED(ListRef) lref, TYPE* item)
{
    List_add_front(lref, (void *) item);
}
