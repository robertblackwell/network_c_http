#define _GNU_SOURCE

#include <c_http/buffer/iobuffer.h>
#include <c_http/alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity )
{
    this->buffer_ptr = this->mem_p = eg_alloc(capacity);
    if(this->mem_p == NULL) goto memerror;
    this->char_p = (char*)this->mem_p;
    this->buffer_capacity = capacity;
    this->buffer_length = this->buffer_capacity;
    this->buffer_remaining = 0;
        return this;
    memerror:
        return NULL;
}

IOBufferRef IOBuffer_new_with_capacity(int capacity)
{
    IOBufferRef pcref = eg_alloc(sizeof(IOBuffer));
    if (pcref == NULL)
        return NULL;
    if(IOBuffer_init(pcref, capacity) == NULL) {
        free(pcref);
    }
    return pcref;
}
IOBufferRef IOBuffer_new()
{
    return IOBuffer_new_with_capacity(IOBUFFER_DEFAULT_CAPACITY);
}

void* IOBuffer_data(const IOBufferRef this)
{
    return this->buffer_ptr;
}
int IOBuffer_data_len(const IOBufferRef this)
{
    return this->buffer_remaining;
}
void* IOBuffer_space(const IOBufferRef this)
{
    return (this->buffer_ptr + this->buffer_remaining);
}
int IOBuffer_space_len(const IOBufferRef this)
{
    return (this->mem_p + this->buffer_capacity) - (this->buffer_ptr + this->buffer_remaining);
}
void IOBuffer_commit(IOBufferRef this, int bytes_used)
{
    this->buffer_remaining = bytes_used;
}
void IOBuffer_consume(IOBufferRef this, int byte_count)
{
    this->buffer_ptr += byte_count;
    // check no off end of buffer
    assert(this->buffer_ptr < (this->mem_p + this->buffer_capacity));
    this->buffer_remaining -= byte_count;
    // check consume did not remove too much
    assert(this->buffer_remaining >= 0);
}
void IOBuffer_destroy(IOBufferRef this)
{
    eg_free(this->mem_p);
}
void IOBuffer_reset(IOBufferRef this)
{
    this->buffer_ptr = this->mem_p;
    this->buffer_remaining = 0;
}
void IOBuffer_free(IOBufferRef* p)
{
    IOBuffer_destroy(*p);
    eg__free(*p);
    *p = NULL;
}
