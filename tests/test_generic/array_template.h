#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// TYPE the type of the table entry - should be castable to and from void* - that it is a pointer or long
// PREFIX the string you want to put in front of Table_ functions to distinguish them for this entry type
//#include "array.h"

typedef struct TYPED(Table_s) TYPED(Table), * TYPED(TableRef);

TYPED(TableRef) TYPED(Table_new)();
void     TYPED(Table_init)(TYPED(TableRef) ref);
TYPE*    PREFIX_Table_get(PREFIX_TableRef this, size_t indx);
void     PREFIX_Table_set(PREFIX_TableRef this, size_t indx, TYPE* data);
size_t   PREFIX_Table_size(PREFIX_TableRef this);
void     PREFIX_Table_push(PREFIX_TableRef this, TYPE* data);

/**
 * Here are a set of macros that do the same thing as the above function prototypes and their impls
 * Use the underlying Table functions with suitable type casting.
 * Again this implementation requires that entries can be cast to and from void*
 * And the Table destroy function needs to know whether to and how to free an entry
 */
#if 0
#define M_PREFIX_Table_init(PREFIX_TableRef this) Table_init((TableRef)this);
#define M_PREFIX_Table_new() (PREFIX_TableRef)Table_new();
#define M_PREFIX_Table_get(PREFIX_TableRef this, size_t idx) (TYPE)Table_get((TableRef)this, idx);
M_PREFIX_Table_set(PREFIX_TableRef this, size_t idx, TYPE data) Table_set((TableRef)this, idx, (void*)data);
M_PREFIX_Table_push(PREFIX_TableRef this, TYPE data) Table_push((TableRef)this, (void*)data);
#endif