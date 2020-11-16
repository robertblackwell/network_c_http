#include <c_http/operation.h>
#include <c_http/oprlist.h>
#include <string.h>
#include <c_http/utils.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dealloc (void **ptr)
{ Opr_free ((Operation**) ptr); }

OprListRef OprList_new ()
{ return (OprListRef) List_new (dealloc); }

void OprList_free (OprListRef *lref_ptr)
{ List_free (lref_ptr); }

int OprList_size (OprListRef lref)
{ return List_size (lref); }

Operation* OprList_first (OprListRef lref)
{ return (Operation*) List_first (lref); }

Operation* OprList_last (OprListRef lref)
{ return (Operation*) List_last (lref); }

Operation* OprList_remove_first (OprListRef lref)
{ return (Operation*) List_remove_first (lref); }

Operation* OprList_remove_last (OprListRef lref)
{ return (Operation*) List_remove_last (lref); }

Operation* OprList_itr_unpack (OprListRef lref, OprListIter iter)
{ return (Operation*) List_itr_unpack (lref, iter); }

OprListIter OprList_iterator (OprListRef lref)
{ return List_iterator (lref); }

OprListIter OprList_itr_next (OprListRef lref, OprListIter iter)
{ return List_itr_next (lref, iter); }

void OprList_itr_remove (OprListRef lref, OprListIter *iter)
{ List_itr_remove (lref, iter); }

void OprList_add_back (OprListRef lref, Operation* item)
{ List_add_back (lref, (void *) item); }

void OprList_add_front (OprListRef lref, Operation* item)
{ List_add_front (lref, (void *) item); }
