#include "helper.h"
inline bool check_header(HeaderListPtr hlist, char* key, char* value)
{
    HeaderLinePtr line = header_list_find(hlist, HEADER_HOST); \
    assert(line != NULL);
    assert( strcmp(Cbuffer_cstr(line->key), HEADER_HOST) == 0);
    assert(strcmp(Cbuffer_cstr(line->value), "ahost") == 0);
    return true;
}
