#include <src/http/kvpair.h>
#include <src/http/header_list.h>
#include <string.h>
#include <src/common/utils.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void kvp_free(void *ptr)
{
    KVPair_free((KVPairRef) ptr);
}

HdrListRef HdrList_new()
{
    return (HdrListRef) List_new(NULL);
}
void HdrList_safe_free(HdrListRef lref)
{
    List_safe_free(lref, kvp_free);
}
int HdrList_size(HdrListRef lref)
{
    return List_size(lref);
}

KVPairRef HdrList_first(HdrListRef lref)
{
    return (KVPairRef) List_first(lref);
}

KVPairRef HdrList_last(HdrListRef lref)
{
    return (KVPairRef) List_last(lref);
}

KVPairRef HdrList_remove_first(HdrListRef lref)
{
    return (KVPairRef) List_remove_first(lref);
}

KVPairRef HdrList_remove_last(HdrListRef lref)
{
    return (KVPairRef) List_remove_last(lref);
}

KVPairRef HdrList_itr_unpack(HdrListRef lref, HdrListIter iter)
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
