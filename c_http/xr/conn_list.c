#include <c_http/xr/conn_list.h>

static void dealloc (void **ptr)
{ XrConnection_free ((XrConnectionRef) ptr); }

XrConnListRef XrConnList_new ()
{ return (XrConnListRef) List_new (dealloc); }

void XrConnList_free (XrConnListRef *lref_ptr)
{ List_free (lref_ptr); }

int XrConnList_size (XrConnListRef lref)
{ return List_size (lref); }

XrConnectionRef XrConnList_first (XrConnListRef lref)
{ return (XrConnectionRef) List_first (lref); }

XrConnectionRef XrConnList_last (XrConnListRef lref)
{ return (XrConnectionRef) List_last (lref); }

XrConnectionRef XrConnList_remove_first (XrConnListRef lref)
{ return (XrConnectionRef) List_remove_first (lref); }

XrConnectionRef XrConnList_remove_last (XrConnListRef lref)
{ return (XrConnectionRef) List_remove_last (lref); }

XrConnectionRef XrConnList_itr_unpack (XrConnListRef lref, XrConnListIter iter)
{ return (XrConnectionRef) List_itr_unpack (lref, iter); }

XrConnListIter XrConnList_iterator (XrConnListRef lref)
{ return List_iterator (lref); }

XrConnListIter XrConnList_itr_next (XrConnListRef lref, XrConnListIter iter)
{ return List_itr_next (lref, iter); }

void XrConnList_itr_remove (XrConnListRef lref, XrConnListIter *iter)
{ List_itr_remove (lref, iter); }

void XrConnList_add_back (XrConnListRef lref, XrConnectionRef item)
{ List_add_back (lref, (void *) item); }

void XrConnList_add_front (XrConnListRef lref, XrConnectionRef item)
{ List_add_front (lref, (void *) item); }
