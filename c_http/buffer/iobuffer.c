#define _GNU_SOURCE

#include <c_http/buffer/iobuffer.h>
#include <c_http/alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define IOB_CHECK_TAG(p) \
do { \
    assert(strcmp((p)->tag, "IOBUF") == 0); \
} while(0);

#define IOB_SET_TAG(p) \
do { \
    sprintf((p)->tag, "%s", "IOBUF"); \
} while(0);

IOBufferRef IOBuffer_init(IOBufferRef this, int capacity )
{
    IOB_SET_TAG(this);
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
IOBufferRef IOBuffer_from_cbuffer(CbufferRef cbuf)
{
    int cap = Cbuffer_size(cbuf);
    IOBufferRef this = IOBuffer_new_with_capacity(cap*2);
    memcpy(IOBuffer_space(this), Cbuffer_data(cbuf), cap);
    IOBuffer_commit(this, cap);
    return this;
}
IOBufferRef IOBuffer_from_buf(char* buf, int len)
{
    int cap = len;
    IOBufferRef this = IOBuffer_new_with_capacity(cap*2);
    memcpy(IOBuffer_space(this), buf, len);
    IOBuffer_commit(this, cap);
    return this;
}
IOBufferRef IOBuffer_from_cstring(char* cstr)
{
    return IOBuffer_from_buf(cstr, strlen(cstr));
}

void* IOBuffer_data(const IOBufferRef this)
{
    IOB_CHECK_TAG(this);
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
    void* x = this->mem_p + this->buffer_capacity;
    assert(this->buffer_ptr <= (this->mem_p + this->buffer_capacity));
    this->buffer_remaining -= byte_count;
    // check consume did not remove too much
    assert(this->buffer_remaining >= 0);
    if(this->buffer_remaining == 0) {
        this->buffer_ptr = this->mem_p;
    }
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
