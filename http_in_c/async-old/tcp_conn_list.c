#include <http_in_c/async/tcp_conn_list.h>
#include <http_in_c/async/tcp_conn.h>

static void dealloc(void **ptr)
{
    TcpConn_free((TcpConnRef) ptr);
}

TcpConnListRef TcpConnList_new()
{
    return (TcpConnListRef) List_new(dealloc);
}

void TcpConnList_dispose(TcpConnListRef *lref_ptr)
{
    List_dispose(lref_ptr);
}

int TcpConnList_size(TcpConnListRef lref)
{
    return List_size(lref);
}

TcpConnRef TcpConnList_first(TcpConnListRef lref)
{
    return (TcpConnRef) List_first(lref);
}

TcpConnRef TcpConnList_last(TcpConnListRef lref)
{
    return (TcpConnRef) List_last(lref);
}

TcpConnRef TcpConnList_remove_first(TcpConnListRef lref)
{
    return (TcpConnRef) List_remove_first(lref);
}

TcpConnRef TcpConnList_remove_last(TcpConnListRef lref)
{
    return (TcpConnRef) List_remove_last(lref);
}

TcpConnRef TcpConnList_itr_unpack(TcpConnListRef lref, TcpConnListIter iter)
{
    return (TcpConnRef) List_itr_unpack(lref, iter);
}

TcpConnListIter TcpConnList_iterator(TcpConnListRef lref)
{
    return List_iterator(lref);
}

TcpConnListIter TcpConnList_itr_next(TcpConnListRef lref, TcpConnListIter iter)
{
    return List_itr_next(lref, iter);
}

void TcpConnList_itr_remove(TcpConnListRef lref, TcpConnListIter* iter_adr)
{
    List_itr_remove(lref, iter_adr);
}

void TcpConnList_add_back(TcpConnListRef lref, TcpConnRef item)
{
    List_add_back(lref, (void *) item);
}

void TcpConnList_add_front(TcpConnListRef lref, TcpConnRef item)
{
    List_add_front(lref, (void *) item);
}
