#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX_TableMINCAP 32

struct PREFIX_Table_s
{
    void* buf;
    size_t capacity;
    size_t size;
};
#ifdef NOTVENEER
/**
 * this version of PREFIX_Table provides a type correct re-implementation. This is very usefull for IDE debugging
 */
void PREFIX_Table_init(TableRef this)
{
this->capacity = PREFIX_Table_MIN_CAP;
this->buf = malloc(sizeof(T) * this->capacity);
this->size = 0;
}

PREFIX_TableRef PREFIX_Table_new()
{
    PREFIX_TableRef this = malloc(sizeof(PREFIX_Table));
    PREFIX_Table_init(this);
    return this;
}

TYPE PREFIX_Table_get(TableRef this, size_t idx)
{
    return vec->buf + idx;
}

void PREFIX_Table_set(TableRef vec, size_t idx, TYPE data)
{
    vec->buf[idx] = data;
}

void PREFIX_Table_push(PREFIX_TableRef this, TYPE data)
{
    if (this->size == this->capacity) {
        this->capacity *= 2;
        this->buf = realloc(this->buf, sizeof(TYPE) * this->capacity);
    }
    PREFIX_Table_set(this, vec->size++, data);
}
#else
/**
 * This version provides a wrapper for the basic Table implementation that corrects the types of
 * arguments and return values but uses the generic implementation via casting.
 * Produces smaller implementations but the data in the table is opaque in debugging.
 *
 * This also only works when table entries can be cast to and from void* nd this requires
 * passing in (somehow) a destructor for the entry type if one is needed
 *
 */

void PREFIX_Table_init(PREFIX_TableRef this)
{
    Table_init((TableRef)this);
}

PREFIX_TableRef PREFIX_Table_new()
{
    return (TableRef)Table_new();
}

TYPE PREFIX_Table_get(PREFIX_TableRef this, size_t idx)
{
    return Table_get((TableRef)this, idx);
}

void PREFIX_Table_set(PREFIX_TableRef this, size_t idx, TYPE data)
{
    Table_set((TableRef)this, idx, (void*)data);
}

void PREFIX_Table_push(PREFIX_TableRef this, TYPE data)
{
    Table_push((TableRef)this, (void*)data);
}

#endif