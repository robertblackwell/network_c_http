#define _GNU_SOURCE

#include <c_eg/buffer/iobuffer.h>
#include <c_eg/alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef IOBUFFER_OPAQUE
typedef struct IOBuffer_s {
    char buffer[1000];
    char*  mem_p;             // always points to the start of buffer
    int    buffer_capacity;   // always holds the size of the buffer
    char*  buffer_ptr;        // points to the start of unused data in buffer
    int    buffer_length;     // same as capacity
    int    buffer_remaining;  // length of datat no consumed

} IOBuffer, *IOBufferRef;
#endif

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity )
{
    this->buffer_ptr = this->mem_p = eg_alloc(capacity);
    if(this->mem_p == NULL) goto memerror;
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
    return IOBuffer_new_with_capacity(IOBUFFER_DEFAULT_CAPACITY)
}

void* IOBUffer_data(IOBufferRef this)
{
    return this->buffer_ptr;
}
int IOBuffer_data_len(IOBufferRef this)
{
    assert(false);
}
void* IOBuffer_space(IOBufferRef this)
{
}
int IOBuffer_space_len(IOBuffer this)
{
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
}
void IOBuffer_reset(IOBufferRef this)
{
    IOBuffer_init(this);
}
void IOBuffer_free(IOBufferRef* p)
{
    IOBuffer_destroy(*p);
    eg__free(*p);
    *p = NULL;
}
