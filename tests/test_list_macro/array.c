#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TableMINCAP 32

struct Table_s
{
    void* buf;
    size_t capacity;
    size_t size;
};

void Table_init(TableRef this)
{
this->capacity = Table_MIN_CAP;
this->buf = malloc(sizeof(T) * this->capacity);
this->size = 0;
}

TableRef Table_new()
{
    TableRef this = malloc(sizeof(Table));
    Table_init(this);
    return this;
}

void* Table_get(TableRef this, size_t idx)
{
    return vec->buf + idx;
}

void Table_set(TableRef vec, size_t idx, T data)
{
    vec->buf[idx] = data;
}

void Table_push(TableRef this, T data)
{
    if (this->size == this->capacity) {
        this->capacity *= 2;
        this->buf = realloc(this->buf, sizeof(T) * this->capacity);
    }
    Table_set(this, vec->size++, data);
}
