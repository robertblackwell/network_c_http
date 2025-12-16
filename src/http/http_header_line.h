#ifndef c_http_http_header_line_h
#define c_http_http_header_line_h
#include <rbl/check_tag.h>
#include <src/common/cbuffer.h>
#include <allocators/alloc.h>
typedef struct HeaderLine_s *HeaderLinePtr;
typedef struct HeaderLine_s {
    RBL_DECLARE_TAG;
    HeaderLinePtr     forward;
    HeaderLinePtr     backward;
    Cbuffer*          key;
    Cbuffer*          value;
    Allocator*        allocator;
    RBL_DECLARE_END_TAG;
} HeaderLine, *HeaderListIter;

// typedef struct HeaderList{
//     int count;
//     HeaderLine* head;
//     HeaderLine* tail;
// } HeaderList, *HeaderListPtr;;

HeaderLinePtr  header_line_new(Cbuffer* key, Cbuffer* value, Allocator* allocator);
HeaderLinePtr  header_line_from_buffer(char* key, int keylen, char* value, int valuelen, Allocator* allocator);
HeaderLinePtr  header_line_from_cstr(char* keycstr, char* valcstr, Allocator* allocator);
void header_line_append_key(HeaderLinePtr hline, char* buf, int len);
void header_line_append_value(HeaderLinePtr hline, char* buf, int len);
void header_line_set_value(HeaderLinePtr hline, Cbuffer* value);
void header_line_free(HeaderLinePtr hline);
void header_line_init(HeaderLinePtr hline, Cbuffer* key, Cbuffer* value, Allocator* allocator);
void header_line_deinit(HeaderLinePtr hline);
void header_line_free(HeaderLinePtr hline);
HeaderLinePtr header_line_copy(HeaderLinePtr src);

#if 0
HeaderListPtr  header_list_new();
void header_list_init(HeaderListPtr hlp);
int  header_list_size(HeaderListPtr lref);

HeaderLinePtr  header_list_first(HeaderListPtr lref);
HeaderLinePtr  header_list_last(HeaderListPtr lref) ;
HeaderLinePtr  header_list_remove_first(HeaderListPtr lref);
HeaderLinePtr  header_list_remove_last(HeaderListPtr lref);
HeaderLinePtr  header_list_itr_unpack(HeaderListPtr lref, HeaderListIter iter);
HeaderListIter header_list_iterator(HeaderListPtr lref);
HeaderListIter header_list_itr_next  (HeaderListPtr lref, HeaderListIter iter);
void  header_list_itr_remove(HeaderListPtr lref, HeaderListIter* iter_addr);

void header_list_add_back(HeaderListPtr lref, HeaderLinePtr line);
void header_list_add_front(HeaderListPtr lref, HeaderLinePtr line);
HeaderListPtr header_list_from_array(const char* raw[][2]);
void header_list_add(HeaderListPtr this, const CbufferRef key, const CbufferRef value);

void header_list_add_many(HeaderListPtr this, CStrPair* pairs[]);
void header_list_add_arr(HeaderListPtr this, const char* ar[][2]);
HeaderLinePtr header_list_find(const HeaderListPtr hlref, const char* key);
void header_list_remove(HeaderListPtr hlref, const char* key);
CbufferRef header_list_serialize(const HeaderListPtr this);
void header_list_add_cbuf(HeaderListPtr this, const CbufferRef key, const CbufferRef value);
void header_list_add_line(HeaderListPtr this, const char* label, int lablen, const char* value, int vallen);
void header_list_add_cstr(HeaderListPtr this, const char* label, const char* value);
#endif
/** @} */

#endif
