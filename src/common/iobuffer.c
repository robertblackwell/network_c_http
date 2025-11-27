#include <common/iobuffer.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define IOBuffer_TAG "IOBUFF"
#include <rbl/check_tag.h>

#define IOB_TERM_CHAR (char)0x00;
#define IOB_FILL_CHAR '+'

typedef struct IOBuffer_s {
    RBL_DECLARE_TAG;
    void*  mem_p;             // always points to the start of buffer
    char*  char_p;
    size_t allocated_capacity; // typically allocate a little more than requested - for a trailing 0x00
    size_t buffer_capacity;   // always holds the size of the buffer
    void*  buffer_ptr;        // points to the start of unused data in buffer
    size_t buffer_length;     // same as capacity
    size_t buffer_remaining;  // length of data not consumed

} IOBuffer, *IOBufferRef;


IOBufferRef IOBuffer_init(IOBufferRef iob, size_t capacity )
{
    RBL_SET_TAG(IOBuffer_TAG, iob);
    iob->allocated_capacity = capacity + 1;
    iob->buffer_ptr = iob->mem_p = malloc(iob->allocated_capacity);
    if(iob->mem_p == NULL) goto memerror;
    iob->char_p = (char*)iob->mem_p;
#ifdef IOB_FILL
    for(int i = 0; i < iob->allocated_capacity; i++) {
        *(char*)(iob->char_p + i) = IOB_FILL_CHAR;
    }
#endif
    iob->buffer_capacity = capacity;
    iob->buffer_length = iob->buffer_capacity;
    iob->buffer_remaining = 0;
        return iob;
    memerror:
        return NULL;
}
void IOBuffer_expand_and_reset(IOBufferRef iob, size_t new_capacity)
{
    if (new_capacity > iob->buffer_capacity) {
        free(iob->mem_p);
        IOBuffer_init(iob, new_capacity);
    }
}
IOBufferRef IOBuffer_new_with_capacity(size_t capacity)
{
    IOBufferRef pcref = malloc(sizeof(IOBuffer));
    if (pcref == NULL) {
        assert(0);
        return NULL;
    }
    if(IOBuffer_init(pcref, capacity) == NULL) {
        free(pcref);
        assert(0);
    }
    return pcref;
}
IOBufferRef IOBuffer_new()
{
    return IOBuffer_new_with_capacity(IOBUFFER_DEFAULT_CAPACITY);
}
IOBufferRef IOBuffer_from_cbuffer(CbufferRef cbuf)
{
    size_t cap = Cbuffer_size(cbuf);
    IOBufferRef iob = IOBuffer_new_with_capacity(cap*2);
    memcpy(IOBuffer_space(iob), Cbuffer_data(cbuf), cap);
    IOBuffer_commit(iob, cap);
    return iob;
}
IOBufferRef IOBuffer_from_buf(char* buf, size_t len)
{
    size_t cap = len;
    IOBufferRef iob = IOBuffer_new_with_capacity(cap*2);
    memcpy(IOBuffer_space(iob), buf, len);
    IOBuffer_commit(iob, cap);
    return iob;
}
IOBufferRef IOBuffer_from_cstring(char* cstr)
{
    return IOBuffer_from_buf(cstr, strlen(cstr));
}
const char* IOBuffer_cstr(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return (const char*) iob->buffer_ptr;
}
IOBufferRef IOBuffer_dup(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    IOBufferRef new = IOBuffer_new_with_capacity(iob->buffer_capacity);
    new->buffer_capacity = iob->buffer_capacity;
    new->allocated_capacity = iob->allocated_capacity;
    new->buffer_remaining = iob->buffer_remaining;
    memcpy(new->mem_p, iob->mem_p, iob->allocated_capacity);
    new->buffer_ptr = (new->mem_p + (iob->buffer_ptr - iob->mem_p));
    new->char_p = new->buffer_ptr;
    return new;
}

void* IOBuffer_data(const IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return iob->buffer_ptr;
}
size_t IOBuffer_data_len(const IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return iob->buffer_remaining;
}
void IOBuffer_expand(IOBufferRef iob, size_t new_capacity)
{
    iob->allocated_capacity = new_capacity + 1;
    long offset = iob->buffer_ptr - iob->mem_p;
    size_t buf_len = iob->buffer_remaining;
    void* mem = iob->mem_p;
    iob->mem_p = realloc(mem, iob->allocated_capacity);
    assert(iob->mem_p != NULL);
    iob->buffer_ptr = iob->mem_p + offset;
    iob->char_p = (char*)iob->mem_p;
#ifdef IOB_FILL
    for(int i = 0; i < iob->allocated_capacity; i++) {
        *(char*)(iob->char_p + i) = IOB_FILL_CHAR;
    }
#endif
    iob->buffer_capacity = new_capacity;
}
void IOBuffer_data_add(IOBufferRef iob, void* p, size_t len)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    void* memp = IOBuffer_space(iob);
    size_t mem_len = IOBuffer_space_len(iob);
    if(mem_len < len) {
        IOBuffer_expand(iob, 2*(iob->allocated_capacity + len));
        memp = IOBuffer_space(iob);
        mem_len = IOBuffer_space_len(iob);
    }
    memcpy(memp, p, len);
    IOBuffer_commit(iob, len);
}

void* IOBuffer_space(const IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    void* tmp = iob->buffer_ptr + iob->buffer_remaining;
    return (iob->buffer_ptr + iob->buffer_remaining);
}
size_t IOBuffer_space_len(const IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return (iob->mem_p + iob->buffer_capacity) - (iob->buffer_ptr + iob->buffer_remaining);
}
void IOBuffer_commit(IOBufferRef iob, size_t bytes_used)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    assert(bytes_used > 0);
    //@TODO  iob looks like a bug test with two successive commits
    // TODO - what happens if the bytes_used parameter is too big
    iob->buffer_remaining += bytes_used;
    *(char*)(iob->mem_p + iob->buffer_remaining) = IOB_TERM_CHAR;
}
void IOBuffer_consolidate_space(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    IOBufferRef tmp = IOBuffer_new_with_capacity(iob->buffer_capacity);
    IOBuffer_data_add(tmp, IOBuffer_data(iob), IOBuffer_data_len(iob));
    void* tmp_mem_p = iob->mem_p;
    iob->mem_p = tmp->mem_p;
    free(tmp_mem_p);
    iob->buffer_ptr = tmp->buffer_ptr;
    iob->buffer_remaining = tmp->buffer_remaining;
    iob->buffer_capacity = tmp->buffer_capacity;
    iob->allocated_capacity = tmp->allocated_capacity;
    iob->char_p = iob->mem_p;
    free(tmp);
}

void IOBuffer_consume(IOBufferRef iob, size_t byte_count)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    iob->buffer_ptr += byte_count;
    // check no off end of buffer
    void* x = iob->mem_p + iob->buffer_capacity;
    // @TODO iob looks like a bug
    assert(iob->buffer_ptr <= (iob->mem_p + iob->buffer_capacity));
    iob->buffer_remaining -= byte_count;
    // check consume did not remove too much
    assert(iob->buffer_remaining >= 0);
    if(iob->buffer_remaining == 0) {
        iob->buffer_ptr = iob->mem_p;
    }
}
void IOBuffer_destroy(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    free(iob->mem_p);
}
void IOBuffer_reset(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    iob->buffer_ptr = iob->mem_p;
    iob->buffer_remaining = 0;
    *(char*)(iob->mem_p + iob->buffer_remaining) = IOB_TERM_CHAR;
}
void IOBuffer_free(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    free(iob->mem_p);
    free(iob);
}
bool IOBuffer_empty(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return iob->buffer_remaining == 0;
}
bool IOBuffer_equal(IOBufferRef a, IOBufferRef b)
{
    RBL_CHECK_TAG(IOBuffer_TAG, a)
    RBL_CHECK_TAG(IOBuffer_TAG, b)
    size_t lena = IOBuffer_data_len(a);
    size_t lenb = IOBuffer_data_len(b);
    void* a_p = IOBuffer_data(a);
    void* b_p = IOBuffer_data(b);
    if( lena != lenb) {
        return false;
    }
    return (strncmp(a_p, b_p, lena) == 0);
}
void* IOBuffer_memptr(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    return iob->mem_p;
}
char IOBuffer_consume_pop_front(IOBufferRef iob)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    char* p = IOBuffer_data(iob);
    char ch = *p;
    IOBuffer_consume(iob, 1);
    return ch;
}
void IOBuffer_commit_push_back(IOBufferRef iob, char ch)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    char tmp = ch;
    IOBuffer_data_add(iob, &tmp, 1);
}
void IOBuffer_sprintf(IOBufferRef iob, const char* fmt, ...)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    va_list args;
    va_start(args, fmt);
    char* buf = IOBuffer_space(iob);
    size_t len1 = IOBuffer_space_len(iob);
    size_t nchars1 = vsnprintf(buf, len1, fmt, args);
    if (nchars1 > len1-1) {
        IOBuffer_expand_and_reset(iob, 2*nchars1);
        size_t len2 = IOBuffer_space_len(iob);
        size_t nchars2 = vsnprintf(buf, len2, fmt, args);
        assert(len2 > nchars2);
        IOBuffer_commit(iob, nchars2);
    } else {
        IOBuffer_commit(iob, nchars1);
    }
    va_end(args);
}
void IOBuffer_append_cstr(IOBufferRef iob, const char* cstr)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    size_t len = 0;
    char* p = (char*)cstr;
    while (*p != '\0') {
        IOBuffer_data_add(iob, (void*)p, 1);
        p++;
    }

}
void IOBuffer_append_buffer(IOBufferRef iob, const char* buf, size_t len)
{
    RBL_CHECK_TAG(IOBuffer_TAG, iob)
    IOBuffer_data_add(iob, (void*)buf, len);
}
