#include <c_eg/header_line.h>
#include <c_eg/headerline_list.h>
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
{ HeaderLine_free ((HeaderLineRef *) ptr); }

HDRListRef HDRList_new ()
{ return (HDRListRef) List_new (dealloc); }

void HDRList_free (HDRListRef *lref_ptr)
{ List_free (lref_ptr); }

int HDRList_size (HDRListRef lref)
{ return List_size (lref); }

HeaderLineRef HDRList_first (HDRListRef lref)
{ return (HeaderLineRef) List_first (lref); }

HeaderLineRef HDRList_last (HDRListRef lref)
{ return (HeaderLineRef) List_last (lref); }

HeaderLineRef HDRList_remove_first (HDRListRef lref)
{ return (HeaderLineRef) List_remove_first (lref); }

HeaderLineRef HDRList_remove_last (HDRListRef lref)
{ return (HeaderLineRef) List_remove_last (lref); }

HeaderLineRef HDRList_itr_unpack (HDRListRef lref, HDRListIter iter)
{ return (HeaderLineRef) List_itr_unpack (lref, iter); }

HDRListIter HDRList_iterator (HDRListRef lref)
{ return List_iterator (lref); }

HDRListIter HDRList_itr_next (HDRListRef lref, HDRListIter iter)
{ return List_itr_next (lref, iter); }

void HDRList_itr_remove (HDRListRef lref, HDRListIter *iter)
{ List_itr_remove (lref, iter); }

void HDRList_add_back (HDRListRef lref, HeaderLineRef item)
{ List_add_back (lref, (void *) item); }

void HDRList_add_front (HDRListRef lref, HeaderLineRef item)
{ List_add_front (lref, (void *) item); }
////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

HDRListIter HDRList_find_iter (HDRListRef hlref, char *key)
{
    HDRListIter result = NULL;
    char *fixed_key = make_upper (key);
    HDRListIter iter = HDRList_iterator (hlref);
    while (iter) {
        HeaderLineRef hlr = HDRList_itr_unpack (hlref, iter);
        char *k = HeaderLine_label (hlr);
        if (strcmp (k, fixed_key) == 0) {
            result = iter;
            break;
        }
        iter = M_HDRList_itr_next(hlref, iter);
    }
    if (fixed_key != NULL) { free (fixed_key); }
    return result;
}

HeaderLineRef HDRList_find (HDRListRef hlref, char *key)
{
    HDRListIter iter = HDRList_find_iter (hlref, key);
    if (iter == NULL) {
        return NULL;
    } else {
        HeaderLineRef hlr = HDRList_itr_unpack (hlref, iter);
        return hlr;
    }
}

void HDRList_remove (HDRListRef hlref, char *key)
{
    HDRListIter iter = HDRList_find_iter (hlref, key);
    if (iter == NULL) {
        return;
    } else {
        HDRList_itr_remove (hlref, &iter);
    }

}

void HDRList_add_cbuf (HDRListRef this, CBufferRef key, CBufferRef value)
{
    char *labptr = CBuffer_data (key);
    int lablen = CBuffer_size (key);
    char *valptr = CBuffer_data (value);
    int vallen = CBuffer_size (value);
    HeaderLineRef hl = HeaderLine_new (labptr, lablen, valptr, vallen);
    M_HDRList_add_back(this, hl);
}

void HDRList_add_line (HDRListRef this, char *label, int lablen, char *value, int vallen)
{
    HeaderLineRef hl_content_type = HeaderLine_new (label, lablen, value, vallen);
    HDRList_add_front (this, hl_content_type);
}

void HDRList_add_cstr (HDRListRef this, char *label, char *value)
{
    int lablen = strlen (label);
    int vallen = strlen (value);
    HeaderLineRef hl_content_type = HeaderLine_new (label, lablen, value, vallen);
    HDRList_add_front (this, hl_content_type);
}

CBufferRef HDRList_serialize (HDRListRef this)
{
    CBufferRef cb = CBuffer_new ();
    ListNodeRef iter = HDRList_iterator (this);
    while (iter != NULL) {
        HeaderLineRef line = HDRList_itr_unpack (this, iter);
        CBuffer_append_cstr (cb, HeaderLine_label (line));
        CBuffer_append_cstr (cb, ": ");
        CBuffer_append_cstr (cb, HeaderLine_value (line));
        CBuffer_append_cstr (cb, "\r\n");
        iter = M_HDRList_itr_next(this, iter);
    }
    return cb;
}
