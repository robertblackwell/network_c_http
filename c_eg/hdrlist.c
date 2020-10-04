#include <c_eg/kvpair.h>
#include <c_eg/hdrlist.h>
#include <string.h>
#include <c_eg/utils.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dealloc (void **ptr)
{ KVPair_free ((KVPair* *) ptr); }

HdrList* HdrList_new ()
{ return (HdrList*) List_new (dealloc); }

void HdrList_free (HdrList* *lref_ptr)
{ List_free (lref_ptr); }

int HdrList_size (HdrList* lref)
{ return List_size (lref); }

KVPair* HdrList_first (HdrList* lref)
{ return (KVPair*) List_first (lref); }

KVPair* HdrList_last (HdrList* lref)
{ return (KVPair*) List_last (lref); }

KVPair* HdrList_remove_first (HdrList* lref)
{ return (KVPair*) List_remove_first (lref); }

KVPair* HdrList_remove_last (HdrList* lref)
{ return (KVPair*) List_remove_last (lref); }

KVPair* HdrList_itr_unpack (HdrList* lref, HdrListIter iter)
{ return (KVPair*) List_itr_unpack (lref, iter); }

HdrListIter HdrList_iterator (HdrList* lref)
{ return List_iterator (lref); }

HdrListIter HdrList_itr_next (HdrList* lref, HdrListIter iter)
{ return List_itr_next (lref, iter); }

void HdrList_itr_remove (HdrList* lref, HdrListIter *iter)
{ List_itr_remove (lref, iter); }

void HdrList_add_back (HdrList* lref, KVPair* item)
{ List_add_back (lref, (void *) item); }

void HdrList_add_front (HdrList* lref, KVPair* item)
{ List_add_front (lref, (void *) item); }
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

HdrListIter HdrList_find_iter (HdrList* hlref, char *key)
{
    HdrListIter result = NULL;
    char *fixed_key = make_upper (key);
    HdrListIter iter = HdrList_iterator (hlref);
    while (iter) {
        KVPair* hlr = HdrList_itr_unpack (hlref, iter);
        char *k = KVPair_label (hlr);
        if (strcmp (k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = HdrList_itr_next (hlref, iter);
    }
    if (fixed_key != NULL) { free (fixed_key); }
    return result;
}

KVPair* HdrList_find (HdrList* hlref, char *key)
{
    HdrListIter iter = HdrList_find_iter (hlref, key);
    if (iter == NULL) {
        return NULL;
    } else {
        KVPair* hlr = HdrList_itr_unpack (hlref, iter);
        return hlr;
    }
}

void HdrList_remove (HdrList* hlref, char *key)
{
    HdrListIter iter = HdrList_find_iter (hlref, key);
    if (iter == NULL) {
        return;
    } else {
        HdrList_itr_remove (hlref, &iter);
    }

}

void HdrList_add_cbuf (HdrList* this, Cbuffer* key, Cbuffer* value)
{
    char *labptr = Cbuffer_data (key);
    int lablen = Cbuffer_size (key);
    char *valptr = Cbuffer_data (value);
    int vallen = Cbuffer_size (value);
    KVPair* hl = KVPair_new (labptr, lablen, valptr, vallen);
    HdrList_add_back (this, hl);
}

void HdrList_add_line (HdrList* this, char *label, int lablen, char *value, int vallen)
{
    KVPair* hl_content_type = KVPair_new (label, lablen, value, vallen);
    HdrList_add_front (this, hl_content_type);
}

void HdrList_add_cstr (HdrList* this, char *label, char *value)
{
    int lablen = strlen (label);
    int vallen = strlen (value);
    KVPair* hl_content_type = KVPair_new (label, lablen, value, vallen);
    HdrList_add_front (this, hl_content_type);
}
void HdrList_add_many(HdrList* this, CStrPair* pairs[])
{
    for(int i = 0; pairs[i] != NULL; i++) {

    }
}

// just to see it update
Cbuffer* HdrList_serialize (HdrList* this)
{
    Cbuffer* cb = Cbuffer_new ();
    ListIterator iter = HdrList_iterator (this);
    while (iter != NULL) {
        KVPair* line = HdrList_itr_unpack (this, iter);
        Cbuffer_append_cstr (cb, KVPair_label (line));
        Cbuffer_append_cstr (cb, ": ");
        Cbuffer_append_cstr (cb, KVPair_value (line));
        Cbuffer_append_cstr (cb, "\r\n");
        iter = HdrList_itr_next (this, iter);
    }
    return cb;
}
