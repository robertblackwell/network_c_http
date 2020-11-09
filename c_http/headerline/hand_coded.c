#include <c_http/kvpair.h>
#include <c_http/hdrlist.h>
#include <string.h>
#include <c_http/utils.h>

__LIST_INCLUDE_H__

HdrListIter HdrList_find_iter(HdrListRef hlref, char* key)
{
    HdrListIter result = NULL;
    char* fixed_key = make_upper(key);
    HdrListIter iter = HdrList_iterator(hlref);
    while(iter) {
        KVPairRef hlr = HdrList_itr_unpack(hlref, iter);
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

KVPairRef HdrList_find(HdrListRef hlref, char* key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return NULL;
    } else {
        KVPairRef hlr = HdrList_itr_unpack(hlref, iter);
        return hlr;
    }
}
void HdrList_remove(HdrListRef hlref, char* key)
{
    HdrListIter iter = HdrList_find_iter(hlref, key);
    if(iter == NULL) {
        return;
    } else {
        HdrList_itr_remove(hlref, &iter);
    }

}
void HdrList_add_cbuf(HdrListRef this, CbufferRef key, CbufferRef value)
{
    char* labptr = Cbuffer_data(key);
    int lablen = Cbuffer_size(key);
    char* valptr = Cbuffer_data(value);
    int vallen = Cbuffer_size(value);
    KVPairRef hl = KVPair_new(labptr, lablen, valptr, vallen);
    M_HdrList_add_back(this, hl);
}
void HdrList_add_line(HdrListRef this, char* label, int lablen, char* value, int vallen)
{
    KVPairRef hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
}
void HdrList_add_cstr(HdrListRef this, char* label, char* value)
{
    int lablen = strlen(label);
    int vallen = strlen(value);
    KVPairRef hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
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
        iter = M_HdrList_itr_next(this, iter);
    }
    return cb;
}
