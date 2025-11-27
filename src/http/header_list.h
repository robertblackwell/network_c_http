#ifndef c_http_http_header_list_h
#define c_http_http_header_list_h
#include <rbl/check_tag.h>
#include <src/common/cbuffer.h>
#include "http_header_line.h"

typedef struct HeaderList{
    RBL_DECLARE_TAG;
    int count;
    HeaderLinePtr head;
    HeaderLinePtr tail;
    Allocator* allocator;
    RBL_DECLARE_END_TAG;
} HeaderList, *HeaderListPtr;;


HeaderListPtr  header_list_new(Allocator* allocator);
HeaderListPtr header_list_from_array(const char* raw[][2], Allocator* allocator);
void header_list_init(HeaderListPtr hlist, Allocator* allocator);

void header_list_free(HeaderListPtr hlist);

void header_list_deinit(HeaderListPtr hlist);
int  header_list_size(HeaderListPtr hlist);

HeaderLinePtr  header_list_first(HeaderListPtr hlist);
HeaderLinePtr  header_list_last(HeaderListPtr hlist) ;
HeaderLinePtr  header_list_remove_first(HeaderListPtr hlist);
HeaderLinePtr  header_list_remove_last(HeaderListPtr hlist);
HeaderLinePtr  header_list_itr_unpack(HeaderListPtr hlist, HeaderListIter iter);
HeaderListIter header_list_iterator(HeaderListPtr hlist);
HeaderListIter header_list_itr_next  (HeaderListPtr hlist, HeaderListIter iter);
void  header_list_itr_remove(HeaderListPtr hlist, HeaderListIter* iter_addr);

void header_list_add_back(HeaderListPtr hlist, HeaderLinePtr line);
void header_list_add_front(HeaderListPtr hlist, HeaderLinePtr line);
void header_list_add(HeaderListPtr hlist, const CbufferRef key, const CbufferRef value);

// void header_list_add_many(HeaderListPtr hlist, CStrPair* pairs[]);
void header_list_add_arr(HeaderListPtr hlist, const char* ar[][2]);
HeaderLinePtr header_list_find(const HeaderListPtr hhlist, const char* key);
void header_list_remove(HeaderListPtr hhlist, const char* key);
CbufferRef header_list_serialize(const HeaderListPtr hlist);
void header_list_add_cbuf(HeaderListPtr hlist, const CbufferRef key, const CbufferRef value);
void header_list_add_line(HeaderListPtr hlist, const char* label, int lablen, const char* value, int vallen);
void header_list_add_cstr(HeaderListPtr hlist, const char* label, const char* value);
void header_list_display(const HeaderListPtr hlist);

/** @} */

#endif
