#include "http_header_line.h"

#include <ctype.h>
#include <string.h>
#include <src/common/utils.h>
#include <common/alloc_malloc.h>

#define HeaderLine_TAG "HDR_TAG"

HeaderLine* header_line_new(Cbuffer* key, Cbuffer* value, Allocator* allocator)
{
    if(allocator == NULL) {
        allocator = default_allocator_create();
    }
    HeaderLine* tmp = allocator_alloc(allocator, sizeof(HeaderLine));
    tmp->allocator = allocator;
    header_line_init(tmp, key, value, allocator);
    return tmp;
}
HeaderLine* header_line_from_buffer(char* key, int keylen, char* value, int valuelen, Allocator* allocator)
{
    if(allocator == NULL) {
        allocator = default_allocator_create();
    }
    CbufferRef k = Cbuffer_new(allocator);
    Cbuffer_append(k, key, keylen);
    CbufferRef v = Cbuffer_new(allocator);
    Cbuffer_append(v, value, valuelen);
    HeaderLine* tmp = header_line_new(k, v, allocator);
    return tmp;
}
HeaderLine* header_line_from_cstr(char* keycstr, char* valuecstr, Allocator* allocator)
{
    if(allocator == NULL) {
        allocator = default_allocator_create();
    }
    CbufferRef k = Cbuffer_new(allocator);
    Cbuffer_append_cstr(k, keycstr);
    CbufferRef v = Cbuffer_new(allocator);
    Cbuffer_append_cstr(v, valuecstr);
    HeaderLine* tmp = header_line_new(k, v, allocator);
    return tmp;
}
void header_line_free(HeaderLine* hdr)
{
    Allocator* allocator = hdr->allocator;
    header_line_deinit(hdr);
    allocator_dealloc(hdr->allocator, hdr);
}
void inplace_toupper(char* cstr)
{
    for (char* p = cstr; *p != '\0'; ++p) {
        *p = toupper(*p);
    }
}
void header_line_init(HeaderLine* hdrline, Cbuffer* key, Cbuffer* value, Allocator* allocator)
{
    RBL_SET_TAG(HeaderLine_TAG, hdrline)
    RBL_SET_END_TAG(HeaderLine_TAG, hdrline)
    assert(sizeof(void*) == sizeof(size_t));
    assert(allocator != NULL);

    inplace_toupper((char*)Cbuffer_cstr(key));
    hdrline->key = key;
    hdrline->value = value;
    hdrline->backward = NULL;
    hdrline->forward = NULL;
}
void header_line_deinit(HeaderLine* hdr)
{
    RBL_SET_TAG(HeaderLine_TAG, hdr)
    RBL_SET_END_TAG(HeaderLine_TAG, hdr)
    Cbuffer_free(hdr->key);
    Cbuffer_free(hdr->value);
}
void header_line_append_key(HeaderLinePtr hline, char* buf, int len)
{
    for (int i = 0; i < len; ++i) {
        char ch = (char)toupper(buf[i]);
        Cbuffer_append(hline->key, &ch, 1);
    }
}
void header_line_append_value(HeaderLinePtr hline, char* buf, int len)
{
    Cbuffer_append(hline->value, buf, len);
}

void header_line_set_value(HeaderLinePtr hline, Cbuffer* value)
{
    CbufferRef tmp = hline->value;
    hline->value = value;
    Cbuffer_free(tmp);
}
