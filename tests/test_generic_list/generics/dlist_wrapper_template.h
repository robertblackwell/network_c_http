#include "dlist.h"

typedef DList PREFIX();
typedef DListRef PREFIX(Ref);
typedef DListIter PREFIX(Iter);
typedef DListIter PREFIX(Iterator);
/**
 * This template produces a double linked list where each node holds a
 * pointer to the type TYPE (that is TYPE*).
 *
 * The functions geberated by this template are thin wrappers (that mainly cast types)
 * rounb the functions provided by the files dlist_warpper.h and dlist_warpper.c
 */

PREFIX(Ref) PREFIX(_new) ();
void  PREFIX(_init)(PREFIX(Ref) lref);
void  PREFIX(_destroy)(PREFIX(Ref) lref);
void  PREFIX(_dispose)(PREFIX(Ref) *lref_adr);
void  PREFIX(_display)(const PREFIX(Ref) this);
int   PREFIX(_size)(const PREFIX(Ref) lref);
void  PREFIX(_add_back)(PREFIX(Ref) lref, TYPE* item);
void  PREFIX(_add_front)(PREFIX(Ref) lref, TYPE* item);
TYPE* PREFIX(_first)(const PREFIX(Ref) lref);
TYPE* PREFIX(_remove_first)(PREFIX(Ref) lref);
TYPE* PREFIX(_last)(const PREFIX(Ref) lref);
TYPE* PREFIX(_remove_last)(PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_iterator)(const PREFIX(Ref) lref);
PREFIX(Iterator) PREFIX(_itr_next)(const PREFIX(Ref) lref, const PREFIX(Iterator) itr);
void PREFIX(_itr_remove) (PREFIX(Ref) lref, PREFIX(Iterator) *itr_adr);
TYPE* PREFIX(_itr_unpack) (PREFIX(Ref) lref, PREFIX(Iterator) itr);
PREFIX(Iterator) PREFIX(_find) (PREFIX(Ref) lref, void* needle);

