#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TableMINCAP 32
#include "array.h"
struct Table_s
{
    size_t capacity;
    size_t size;
    void* buf;
};

void Table_init(TableRef this)
{
this->capacity = TableMINCAP;
this->buf = malloc(sizeof(void*) * this->capacity);
this->size = 0;
}

TableRef Table_new(int capacity)
{
    TableRef this = malloc(sizeof(Table));
    Table_init(this);
    return this;
}

void* Table_get(TableRef this, size_t idx)
{
    void** p = (this->buf) + idx;
    return *p;
}

void Table_set(TableRef vec, size_t idx, void* data)
{
    void** p = (vec->buf) + idx;
    *p = data;
}

void Table_push(TableRef this, void* data)
{
    if (this->size == this->capacity) {
        this->capacity *= 2;
        this->buf = realloc(this->buf, sizeof(void*) * this->capacity);
    }
    Table_set(this, this->size++, data);
}
