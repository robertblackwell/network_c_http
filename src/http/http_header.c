#include "http_header.h"

#include <ctype.h>
#include <string.h>
#include <src/common/utils.h>

#define HttpHeader_TAG "HDR_TAG"
static char* tag_hdr_offset = "LiNeOfFs";
static char* tag_hdr_lines = "LiNeBUFf";

void set_offset_tags(HttpHeaders* hdr)
{
    assert(hdr != NULL);
    assert(strlen(tag_hdr_offset) == 8);
    strncpy(hdr->line_offsets_tag, tag_hdr_offset, 8);
    strncpy(hdr->line_offsets_end_tag, tag_hdr_offset, 8);
}
void check_offset_tag(HttpHeaders* hdr)
{
    assert(hdr != NULL);
    assert(strlen(tag_hdr_offset) == 8);
    assert(0 == strncmp(hdr->line_offsets_tag, tag_hdr_offset, 8));
    assert(0 == strncmp(hdr->line_offsets_end_tag, tag_hdr_offset, 8));
}
void set_lines_tags(HttpHeaders* hdr)
{
    assert(hdr != NULL);
    assert(strlen(tag_hdr_lines) == 8);
    strncpy(hdr->lines_buffer_tag, tag_hdr_lines, 8);
    strncpy(hdr->lines_buffer_end_tag, tag_hdr_lines, 8);
}
void check_lines_tags(HttpHeaders* hdr)
{
    assert(hdr != NULL);
    assert(strlen(tag_hdr_lines) == 8);
    assert(0 == strncmp(hdr->lines_buffer_tag, tag_hdr_lines, 8));
    assert(0 == strncmp(hdr->lines_buffer_end_tag, tag_hdr_lines, 8));
}

HttpHeaders* http_header_new(size_t nbr_lines, size_t line_buffer_size)
{
    HttpHeaders* tmp = malloc(sizeof(HttpHeaders));
    http_header_init(tmp, nbr_lines, line_buffer_size);
    return tmp;
}
void http_header_free(HttpHeaders* hdr)
{
    http_header_deinit(hdr);
    free(hdr);
}
void http_header_init(HttpHeaders* hdr, size_t nbr_lines, size_t line_buffer_size)
{
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    assert(sizeof(void*) == sizeof(size_t));
    hdr->line_offsets_raw = malloc(sizeof(size_t) * (nbr_lines + 2));
    assert(hdr->line_offsets_raw != NULL);

    hdr->line_offsets_start = (void*)(hdr->line_offsets_raw + sizeof(void*));
    hdr->line_offsets_end = hdr->line_offsets_raw + (sizeof(size_t) * (nbr_lines + 2));
    hdr->line_offsets_tag = hdr->line_offsets_raw;
    hdr->line_offsets_end_tag = hdr->line_offsets_end - sizeof(size_t);
    set_offset_tags(hdr);

    // ensure allocate multiple of sizeof(void*)
    size_t j = (line_buffer_size / sizeof(void*));
    size_t k = j * sizeof(void*);
    assert(k <= line_buffer_size);
    size_t n = (k < line_buffer_size) ? (j+1)*sizeof(void*) : j * sizeof(void*);
    hdr->lines_buffer_raw = malloc(n + 16);
    assert(hdr->lines_buffer_raw != NULL);
    hdr->lines_buffer_start = hdr->lines_buffer_raw + 8;
    hdr->lines_buffer_end = hdr->lines_buffer_raw + n + 16;
    hdr->lines_buffer_tag = hdr->lines_buffer_raw;
    hdr->lines_buffer_end_tag = hdr->lines_buffer_end - 8;
    hdr->lines_buffer_next = hdr->lines_buffer_start;
    set_lines_tags(hdr);
    hdr->size = 0;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
    check_lines_tags(hdr);
    check_offset_tag(hdr);
}
void http_header_deinit(HttpHeaders* hdr)
{
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    free(hdr->lines_buffer_raw);
    free(hdr->line_offsets_raw);
}
size_t http_header_size(HttpHeaders* header)
{
    return header->size;
}
void http_header_set_pair(HttpHeaders* hdrs, const char* key, int klen, const char* value, int vlen)
{

}

void http_header_set_key(HttpHeaders* hdr, const char* key, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(!hdr->waiting_for_value);
    hdr->waiting_for_value = true;
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        assert(isprint(*key) && (*key != ' ') );
        *p = *key;
        p++; key++;
    }
    *p = ':'; p++;
    size_t* q = hdr->line_offsets_start;
    q[hdr->size] = hdr->next_offset;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
}
void http_header_set_value(HttpHeaders* hdr, const char* value, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(hdr->waiting_for_value);
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        *p = *value;
        p++; value++;
    }
    *p = '\r'; p++;
    *p = '\n';p++;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
    hdr->size++;
    hdr->waiting_for_value = false;
}
void append_line_chars(HttpHeaders* hdr, const char* key, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        assert(isprint(*key) && (*key != ' ') );
        *p = *key;
        p++; key++;
    }
    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
}
void http_header_begin_key(HttpHeaders* hdr, const char* key, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(!hdr->waiting_for_value);
    hdr->waiting_for_value = true;
    hdr->current_line_ptr = hdr->lines_buffer_next;
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        assert(isprint(*key) && (*key != ' ') );
        *p = *key;
        p++; key++;
    }
    // *p = ':'; p++;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;

    size_t* q = hdr->line_offsets_start;
    q[hdr->size] = hdr->next_offset;
}
void http_header_append_key(HttpHeaders* hdr, const char* key, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(!hdr->waiting_for_value);
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        assert(isprint(*key) && (*key != ' ') );
        *p = *key;
        p++; key++;
    }
    // *p = ':'; p++;
    // size_t* q = hdr->line_offsets_start;
    // q[hdr->size] = hdr->next_offset;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
}
void http_header_beging_value(HttpHeaders* hdr, const char* value, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(!hdr->waiting_for_value);
    hdr->waiting_for_value = true;
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    *p = ':'; p++;
    while (len-- > 0) {
        *p = *value;
        p++; value++;
    }
    // *p = '\r'; p++;
    // *p = '\n';p++;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
    // hdr->size++;
    // hdr->waiting_for_value = false;
}
void http_header_append_value(HttpHeaders* hdr, const char* value, int len)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(hdr->waiting_for_value);
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (len+1 > remaining_buffer) {
        assert(0);
    }
    while (len-- > 0) {
        *p = *value;
        p++; value++;
    }

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
}
void http_header_end_line(HttpHeaders* hdr)
{
    assert(hdr != NULL);
    RBL_SET_TAG(HttpHeader_TAG, hdr)
    RBL_SET_END_TAG(HttpHeader_TAG, hdr)
    check_offset_tag(hdr);
    check_lines_tags(hdr);
    assert(hdr->waiting_for_value);
    char* p = (char*)hdr->lines_buffer_start + hdr->next_offset;
    size_t remaining_buffer = hdr->lines_buffer_end - p;
    if (2+1 > remaining_buffer) {
        assert(0);
    }
    *p = '\r'; p++;
    *p = '\n';p++;

    hdr->lines_buffer_next = p;
    hdr->next_offset = hdr->lines_buffer_next - hdr->lines_buffer_start;
    hdr->size++;
    hdr->waiting_for_value = false;
}

#if 0
KVPairRef HdrList_first(HttpHeader* header)
{
    return ;
}

KVPairRef HdrList_last(HttpHeader* header)
{
    return (KVPairRef) List_last(lref);
}

KVPairRef HdrList_remove_first(HttpHeader* header)
{
    return (KVPairRef) List_remove_first(lref);
}

KVPairRef HdrList_remove_last(HttpHeader* header)
{
    return (KVPairRef) List_remove_last(lref);
}

KVPairRef HdrList_itr_unpack(HttpHeader* header, HdrListIter iter)
{
    return (KVPairRef) List_itr_unpack(lref, iter);
}

HdrListIter HdrList_iterator(HdrListRef lref)
{
    return List_iterator(lref);
}

HdrListIter HdrList_itr_next(HdrListRef lref, HdrListIter iter)
{
    return List_itr_next(lref, iter);
}

void HdrList_itr_remove(HdrListRef lref, HdrListIter *iter)
{
    List_itr_remove(lref, iter);
}

void HdrList_add_back(HdrListRef lref, KVPairRef item)
{
    List_add_back(lref, (void *) item);
}

void HdrList_add_front(HdrListRef lref, KVPairRef item)
{
    List_add_front(lref, (void *) item);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

HdrListRef HdrList_from_array(const char* ar[][2])
{
    HdrListRef tmp = HdrList_new();
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        HdrList_add_cstr(tmp, k, v);
    }
    return tmp;
}
void HdrList_add_arr(HdrListRef this, const char* ar[][2])
{
    const char* k;
    const char* v;
    for(int row = 0; ar[row][0] != NULL ; row++) {
        k = ar[row][0];
        v = ar[row][1];
        HdrList_add_cstr(this, k, v);
    }

}

HdrListIter HdrList_find_iter(const HdrListRef hlref, const char *key)
{
    HdrListIter result = NULL;
    char *fixed_key = make_upper(key);
    HdrListIter iter = HdrList_iterator(hlref);
    while(iter) {
        KVPairRef hlr = HdrList_itr_unpack(hlref, iter);
        char *k = KVPair_label(hlr);
        if(strcmp(k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = HdrList_itr_next(hlref, iter);
    }
    if(fixed_key != NULL) { free(fixed_key); }
    return result;
}

KVPairRef HdrList_find(const HdrListRef hlref, const char *key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return NULL;
    } else {
        KVPairRef hlr = HdrList_itr_unpack(hlref, iter);
        return hlr;
    }
}

void HdrList_remove(HdrListRef hlref, const char *key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return;
    } else {
        HdrList_itr_remove(hlref, &iter);
    }

}

void HdrList_add_cbuf(HdrListRef this, const CbufferRef key, const CbufferRef value)
{
    char *labptr = Cbuffer_data(key);
    int lablen = Cbuffer_size(key);
    char *valptr = Cbuffer_data(value);
    int vallen = Cbuffer_size(value);
    KVPairRef hl = KVPair_new(labptr, lablen, valptr, vallen);
    HdrList_add_back(this, hl);
}

void HdrList_add_line(HdrListRef this, const char *label, int lablen, const char *value, int vallen)
{
    KVPairRef hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
}

void HdrList_add_cstr(HdrListRef this, const char *label, const char *value)
{
    int lablen = strlen(label);
    int vallen = strlen(value);
    KVPairRef hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_back(this, hl_content_type);
}

void HdrList_add_many(HdrListRef this, CStrPair *pairs[])
{
    for(int i = 0; pairs[i] != NULL; i++) {

    }
}

// just to see it update
CbufferRef HdrList_serialize(HdrListRef this)
{
    CbufferRef cb = Cbuffer_new();
    ListIterator iter = HdrList_iterator(this);
    while(iter != NULL) {
        KVPairRef line = HdrList_itr_unpack(this, iter);
        Cbuffer_append_cstr(cb, KVPair_label(line));
        Cbuffer_append_cstr(cb, ": ");
        Cbuffer_append_cstr(cb, KVPair_value(line));
        Cbuffer_append_cstr(cb, "\r\n");
        iter = HdrList_itr_next(this, iter);
    }
    return cb;
}
#endif