#include <c_http/common/list.h>
typedef List TYPED(List);
typedef ListRef TYPED(ListRef);
typedef ListIter TYPED(ListIter);
typedef ListIter TYPED(ListIterator);


TYPED(ListRef) TYPED(List_new) ();
void  TYPED(List_init)(TYPED(ListRef) lref);
void  TYPED(List_destroy)(TYPED(ListRef) lref);
void  TYPED(List_dispose)(TYPED(ListRef) *lref_adr);
void  TYPED(List_display)(const TYPED(ListRef) this);
int   TYPED(List_size)(const TYPED(ListRef) lref);
void  TYPED(List_add_back)(TYPED(ListRef) lref, TYPE* item);
void  TYPED(List_add_front)(TYPED(ListRef) lref, TYPE* item);
TYPE* TYPED(List_first)(const TYPED(ListRef) lref);
TYPE* TYPED(List_remove_first)(TYPED(ListRef) lref);
TYPE* TYPED(List_last)(const TYPED(ListRef) lref);
TYPE* TYPED(List_remove_last)(TYPED(ListRef) lref);
TYPED(ListIterator) TYPED(List_iterator)(const TYPED(ListRef) lref);
TYPED(ListIterator) TYPED(List_itr_next)(const TYPED(ListRef) lref, const TYPED(ListIterator) itr);
void TYPED(List_itr_remove) (TYPED(ListRef) lref, TYPED(ListIterator) *itr_adr);
TYPE* TYPED(List_itr_unpack) (TYPED(ListRef) lref, TYPED(ListIterator) itr);
TYPED(ListIterator) TYPED(List_find) (TYPED(ListRef) lref, void* needle);

