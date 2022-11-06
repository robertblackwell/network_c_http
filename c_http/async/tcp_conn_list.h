#ifndef c_http_xr_conn_list_h
#define c_http_xr_conn_list_h

#include "types.h"

/**
 * @addtogroup group_conn_list
 * @{
 */

TcpConnListRef TcpConnList_new();
void TcpConnList_dispose();

int TcpConnList_size (TcpConnListRef lref);

TcpConnRef TcpConnList_first (TcpConnListRef lref);

TcpConnRef TcpConnList_last (TcpConnListRef lref);

TcpConnRef TcpConnList_remove_first (TcpConnListRef lref);

TcpConnRef TcpConnList_remove_last (TcpConnListRef lref);

TcpConnRef TcpConnList_itr_unpack (TcpConnListRef lref, TcpConnListIter iter);

TcpConnListIter TcpConnList_iterator (TcpConnListRef lref);

TcpConnListIter TcpConnList_itr_next (TcpConnListRef lref, TcpConnListIter iter);

void TcpConnList_itr_remove (TcpConnListRef lref, TcpConnListIter* iter_addr);

void TcpConnList_add_back (TcpConnListRef lref, TcpConnRef item);

void TcpConnList_add_front (TcpConnListRef lref, TcpConnRef item);


/** @} */
#endif