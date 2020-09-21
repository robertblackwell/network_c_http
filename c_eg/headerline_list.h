#ifndef c_ceg_headerline_list_h
#define c_ceg_headerline_list_h
#include <c_eg/header_line.h>
#include <c_eg/buffer/contig_buffer.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <c_eg/list.h>
typedef ListRef HDRListRef;
typedef ListNodeRef HDRListIter, ListIter;


#define M_HDRList_new() List_new(dealloc)
#define M_HDRList_free(lref) List_free(lref)
#define M_HDRList_first(lref) (HeaderLineListRef)List_first(lref)
#define M_HDRList_size(lref) (HeaderLineListRef)List_size(lref)
#define M_HDRList_last(lref) (HeaderLineListRef)List_last(lref)
#define M_HDRList_remove_first(lref) (HeaderLineListRef)List_remove_first(lref)
#define M_HDRList_remove_last(lref) (HeaderLineListRef)List_remove_last(lref)
#define M_HDRList_itr_unpack(lref, iter) (HeaderLineListRef)List_itr_unpack(lref, iter)
#define M_HDRList_iterator(lref) List_iterator(lref)
#define M_HDRList_itr_next(lref, iter) List_itr_next(lref, iter)
#define M_HDRList_itr_remove(lref, itr)

#define M_HDRList_add_back(lref, item) List_add_back(lref, (void*)item);
#define M_HDRList_add_front(lref, item) List_add_back(lref, (void*)item);


HDRListRef  HDRList_new();
void HDRList_free(HDRListRef* lref_ptr) ;
int  HDRList_size(HDRListRef lref);

HeaderLineRef  HDRList_first(HDRListRef lref);
HeaderLineRef  HDRList_last(HDRListRef lref) ;
HeaderLineRef  HDRList_remove_first(HDRListRef lref);
HeaderLineRef  HDRList_remove_last(HDRListRef lref);
HeaderLineRef  HDRList_itr_unpack(HDRListRef lref, HDRListIter iter);
HDRListIter HDRList_iterator(HDRListRef lref);
HDRListIter HDRList_itr_next  (HDRListRef lref, HDRListIter iter);
void               HDRList_itr_remove(HDRListRef lref, HDRListIter* iter);

void HDRList_add_back(HDRListRef lref, HeaderLineRef item);
void HDRList_add_front(HDRListRef lref, HeaderLineRef item);

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

void HDRList_add(HDRListRef this, CBufferRef key, CBufferRef value);
HeaderLineRef HDRList_find(HDRListRef hlref, char* key);
void HDRList_remove(HDRListRef hlref, char* key);
CBufferRef HDRList_serialize(HDRListRef this);
#endif
