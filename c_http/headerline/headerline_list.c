#include <c_http/kvpair.h>
#include <c_http/hdrlist.h>
#include <string.h>
#include <c_http/utils.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dealloc(void** ptr) {KVPair_free((KVPairRef*) ptr);}

HdrListRef  HdrList_new() {return (HdrListRef)List_new(dealloc);}
void         HdrList_free(HdrListRef* lref_ptr) {List_free(lref_ptr);}
int          HdrList_size(HdrListRef lref) {return List_size(lref);}
KVPairRef  HdrList_first(HdrListRef lref) { return (KVPairRef)List_first(lref);}
KVPairRef  HdrList_last(HdrListRef lref)  { return (KVPairRef)List_last(lref);}
KVPairRef  HdrList_remove_first(HdrListRef lref) { return (KVPairRef)List_remove_first(lref);}
KVPairRef  HdrList_remove_last(HdrListRef lref) { return (KVPairRef)List_remove_last(lref);}
KVPairRef  HdrList_itr_unpack(HdrListRef lref, HdrListIter iter) { return (KVPairRef)List_itr_unpack(lref, iter);}
HdrListIter HdrList_iterator(HdrListRef lref) { return List_iterator(lref);}
HdrListIter HdrList_itr_next(HdrListRef lref, HdrListIter iter) { return List_itr_next(lref, iter);}
void               HdrList_itr_remove(HdrListRef lref, HdrListIter* iter) { List_itr_remove(lref, iter);}

void HdrList_add_back(HdrListRef lref, KVPairRef item) {List_add_back(lref, (void*)item);}
void HdrList_add_front(HdrListRef lref, KVPairRef item) {List_add_front(lref, (void*)item);}
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

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
ListNode* iter = HdrList_iterator(this);
while(iter != NULL) {
KVPairRef line = HdrList_itr_unpack(this, iter);
Cbuffer_append_cstr(cb, KVPair_label(line));
Cbuffer_append_cstr(cb, ": ");
Cbuffer_append_cstr(cb, KVPair_value(line));
Cbuffer_append_cstr(cb, "rn");
iter = M_HdrList_itr_next(this, iter);
}
return cb;
}
