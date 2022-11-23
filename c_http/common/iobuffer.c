#define _GNU_SOURCE

#include <c_http/common/iobuffer.h>
#include <c_http/common/alloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define TYPE IOBuffer
#define IOBuffer_TAG "IOBUFF"
#include <c_http/check_tag.h>
#undef TYPE
#define IOBUFFER_DECLARE_TAG DECLARE_TAG(IOBuffer)
#define IOBUFFER_CHECK_TAG(p) CHECK_TAG(IOBuffer, p)
#define IOBUFFER_SET_TAG(p) SET_TAG(IOBuffer, p)


//#define IOB_FILL
#ifdef IOB_FILL
#define IOB_TERM_CHAR '?';
#define IOB_FILL_CHAR '+'
#else
#define IOB_TERM_CHAR (char)0x00;
#define IOB_FILL_CHAR '+'
#endif

typedef struct IOBuffer_s {
    IOBUFFER_DECLARE_TAG;
    void*  mem_p;             // always points to the start of buffer
    char*  char_p;
    int    allocated_capacity; // typically allocate a little more than requested - for a trailing 0x00
    int    buffer_capacity;   // always holds the size of the buffer
    void*  buffer_ptr;        // points to the start of unused data in buffer
    int    buffer_length;     // same as capacity
    int    buffer_remaining;  // length of data no consumed

} IOBuffer, *IOBufferRef;


IOBufferRef IOBuffer_init(IOBufferRef this, int capacity )
{
    IOBUFFER_SET_TAG(this);
    this->allocated_capacity = capacity + 1;
    this->buffer_ptr = this->mem_p = eg_alloc(this->allocated_capacity);
    if(this->mem_p == NULL) goto memerror;
    this->char_p = (char*)this->mem_p;
#ifdef IOB_FILL
    for(int i = 0; i < this->allocated_capacity; i++) {
        *(char*)(this->char_p + i) = IOB_FILL_CHAR;
    }
#endif
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
const char* IOBuffer_cstr(IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return (const char*) this->buffer_ptr;
}
IOBufferRef IOBuffer_dup(IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    IOBufferRef new = IOBuffer_new_with_capacity(this->buffer_capacity);
    new->buffer_capacity = this->buffer_capacity;
    new->allocated_capacity = this->allocated_capacity;
    new->buffer_remaining = this->buffer_remaining;
    memcpy(new->mem_p, this->mem_p, this->allocated_capacity);
    new->buffer_ptr = (new->mem_p + (this->buffer_ptr - this->mem_p));
    new->char_p = new->buffer_ptr;
    return new;
}

void* IOBuffer_data(const IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return this->buffer_ptr;
}
int IOBuffer_data_len(const IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return this->buffer_remaining;
}
void IOBuffer_data_add(IOBufferRef this, void* p, int len)
{
    IOBUFFER_CHECK_TAG(this)
    void* memp = IOBuffer_space(this);
    int mem_len = IOBuffer_space_len(this);
    assert(mem_len >= len);
    memcpy(memp, p, len);
    IOBuffer_commit(this, len);
}

void* IOBuffer_space(const IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return (this->buffer_ptr + this->buffer_remaining);
}
int IOBuffer_space_len(const IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return (this->mem_p + this->buffer_capacity) - (this->buffer_ptr + this->buffer_remaining);
}
void IOBuffer_commit(IOBufferRef this, int bytes_used)
{
    IOBUFFER_CHECK_TAG(this)
    assert(bytes_used > 0);
    //@TODO  this looks like a bug test with two successive commits
    // TODO - what happens if the bytes_used parameter is too big
    this->buffer_remaining += bytes_used;
    *(char*)(this->mem_p + this->buffer_remaining) = IOB_TERM_CHAR;
}
void IOBuffer_consolidate_space(IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    IOBufferRef tmp = IOBuffer_new_with_capacity(this->buffer_capacity);
    IOBuffer_data_add(tmp, IOBuffer_data(this), IOBuffer_data_len(this));
    void* tmp_mem_p = this->mem_p;
    this->mem_p = tmp->mem_p;
    free(tmp_mem_p);
    this->buffer_ptr = tmp->buffer_ptr;
    this->buffer_remaining = tmp->buffer_remaining;
    this->buffer_capacity = tmp->buffer_capacity;
    this->allocated_capacity = tmp->allocated_capacity;
    this->char_p = this->mem_p;
    free(tmp);
}

void IOBuffer_consume(IOBufferRef this, int byte_count)
{
    IOBUFFER_CHECK_TAG(this)
    this->buffer_ptr += byte_count;
    // check no off end of buffer
    void* x = this->mem_p + this->buffer_capacity;
    // @TODO this looks like a bug
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
    IOBUFFER_CHECK_TAG(this)
    eg_free(this->mem_p);
}
void IOBuffer_reset(IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    this->buffer_ptr = this->mem_p;
    this->buffer_remaining = 0;
}
void IOBuffer_dispose(IOBufferRef* p)
{
    IOBUFFER_CHECK_TAG(*p)
    IOBuffer_destroy(*p);
    eg__free(*p);
    *p = NULL;
}
bool IOBuffer_equal(IOBufferRef a, IOBufferRef b)
{
    IOBUFFER_CHECK_TAG(a)
    IOBUFFER_CHECK_TAG(b)
    int lena = IOBuffer_data_len(a);
    int lenb = IOBuffer_data_len(b);
    void* a_p = IOBuffer_data(a);
    void* b_p = IOBuffer_data(b);
    if( lena != lenb) {
        return false;
    }
    return (strncmp(a_p, b_p, lena) == 0);
}
void* IOBuffer_memptr(IOBufferRef this)
{
    IOBUFFER_CHECK_TAG(this)
    return this->mem_p;
}
