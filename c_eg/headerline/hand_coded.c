#include <c_eg/kvpair.h>
#include <c_eg/hdrlist.h>
#include <string.h>
#include <c_eg/utils.h>

__LIST_INCLUDE_H__

HdrListIter HdrList_find_iter(HdrList* hlref, char* key)
{
    HdrListIter result = NULL;
    char* fixed_key = make_upper(key);
    HdrListIter iter = HdrList_iterator(hlref);
    while(iter) {
        KVPair* hlr = HdrList_itr_unpack(hlref, iter);
        char* k = KVPair_label(hlr);
        if(strcmp(k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = M_HdrList_itr_next(hlref, iter);
    }
    if(fixed_key != NULL) free(fixed_key);
    return result;
}

KVPair* HdrList_find(HdrList* hlref, char* key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return NULL;
    } else {
        KVPair* hlr = HdrList_itr_unpack(hlref, iter);
        return hlr;
    }
}
void HdrList_remove(HdrList* hlref, char* key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return;
    } else {
        HdrList_itr_remove(hlref, &iter);
    }

}
void HdrList_add_cbuf(HdrList* this, CBufferRef key, CBufferRef value)
{
    char* labptr = CBuffer_data(key);
    int lablen = CBuffer_size(key);
    char* valptr = CBuffer_data(value);
    int vallen = CBuffer_size(value);
    KVPair* hl = KVPair_new(labptr, lablen, valptr, vallen);
    M_HdrList_add_back(this, hl);
}
void HdrList_add_line(HdrList* this, char* label, int lablen, char* value, int vallen)
{
    KVPair* hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
}
void HdrList_add_cstr(HdrList* this, char* label, char* value)
{
    int lablen = strlen(label);
    int vallen = strlen(value);
    KVPair* hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
}
// just to see it update
CBufferRef HdrList_serialize(HdrList* this)
{
    CBufferRef cb = CBuffer_new();
    ListIterator iter = HdrList_iterator(this);
    while(iter != NULL) {
        KVPair* line = HdrList_itr_unpack(this, iter);
        CBuffer_append_cstr(cb, KVPair_label(line));
        CBuffer_append_cstr(cb, ": ");
        CBuffer_append_cstr(cb, KVPair_value(line));
        CBuffer_append_cstr(cb, "\r\n");
        iter = M_HdrList_itr_next(this, iter);
    }
    return cb;
}
