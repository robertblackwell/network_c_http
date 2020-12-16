#include <c_http/runloop/conn_list.h>

static void dealloc(void **ptr)
{
    XrConn_free((XrConnRef) ptr);
}

XrConnListRef XrConnList_new()
{
    return (XrConnListRef) List_new(dealloc);
}

void XrConnList_dispose(XrConnListRef *lref_ptr)
{
    List_dispose(lref_ptr);
}

int XrConnList_size(XrConnListRef lref)
{
    return List_size(lref);
}

XrConnRef XrConnList_first(XrConnListRef lref)
{
    return (XrConnRef) List_first(lref);
}

XrConnRef XrConnList_last(XrConnListRef lref)
{
    return (XrConnRef) List_last(lref);
}

XrConnRef XrConnList_remove_first(XrConnListRef lref)
{
    return (XrConnRef) List_remove_first(lref);
}

XrConnRef XrConnList_remove_last(XrConnListRef lref)
{
    return (XrConnRef) List_remove_last(lref);
}

XrConnRef XrConnList_itr_unpack(XrConnListRef lref, XrConnListIter iter)
{
    return (XrConnRef) List_itr_unpack(lref, iter);
}

XrConnListIter XrConnList_iterator(XrConnListRef lref)
{
    return List_iterator(lref);
}

XrConnListIter XrConnList_itr_next(XrConnListRef lref, XrConnListIter iter)
{
    return List_itr_next(lref, iter);
}

void XrConnList_itr_remove(XrConnListRef lref, XrConnListIter* iter_adr)
{
    List_itr_remove(lref, iter_adr);
}

void XrConnList_add_back(XrConnListRef lref, XrConnRef item)
{
    List_add_back(lref, (void *) item);
}

void XrConnList_add_front(XrConnListRef lref, XrConnRef item)
{
    List_add_front(lref, (void *) item);
}
