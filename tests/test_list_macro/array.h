#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define vector_MIN_CAP 32

typedef struct Table_s Table, *TableRef;

TableRef Table_new();
void     Table_init(TableRef ref);
void*    Table_get(TableRef this, size_t indx);
void     Table_set(TableRef this, size_t indx, void* data);
size_t   Table_size(TableRef this);
void     Table_push(TableRef this, void* data);
