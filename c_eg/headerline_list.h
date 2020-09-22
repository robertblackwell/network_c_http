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

///
/// Create a new HeaderLine instance from key and value and add
/// that HeaderLine to the HDRList.
///
/// The content of key and value are copied into the new HeaderLine instance
/// and hence ownership of key and value remain with the caller
///
/// param this HDRListRef
/// param key CBufferRef
/// param CBufferRef
/// return void
///
void HDRList_add(HDRListRef this, CBufferRef key, CBufferRef value);

///
/// Find a HeaderLine in a HDRList by key/label value
///
/// param hlref HDRListRef
/// param key char*
/// return HeaderLineRef or NULL
/// NULL on not found
/// NOTE: If found the HeaderLineRef returns is still owned by the HDRList
/// do not call HeaderLine_free() on the returned value
///
HeaderLineRef HDRList_find(HDRListRef hlref, char* key);

///
/// Remove a HeaderLine from the HDRList by key/label
///
/// param hlref HDRListRef
/// param key char*
///
void HDRList_remove(HDRListRef hlref, char* key);

/// Serialize a header list into a CBufferRef
/// param this HDRListRef
/// return A serialized version of the header list as a CBuffer.
/// NOTE: ownership of the CBuffer is transfered to the caller
CBufferRef HDRList_serialize(HDRListRef this);

///
/// Adds a new header line to the list
///
/// param this HDRLineRef A ref for the list being added to
/// param key CBufferRef holding the key or label for the header line.
/// param value CBufferRef holding the value for the header line
///
/// The content of the CBufferRef are copied into the header line so the caller is free
/// to deal with the two CBufferRef as they wish.
///
///
void HDRList_add_cbuf(HDRListRef this, CBufferRef key, CBufferRef value);

///
/// Adds a new header line to the list
///
/// param this HDRLineRef A ref for the list being added to
/// param label char* A pointer to a string buffer with the key or label
/// param lablen int the length of the key or label, no assumptions about zero terminated
///param value char* pointer to a string buffer holding the value of the header line
/// param vallen int the length of the value, no assumptions about zero terminated
///
/// The content of the char* are copied into the header line so the caller is free
/// to deal with the two char* buffers as they wish.
///
////
void HDRList_add_line(HDRListRef this, char* label, int lablen, char* value, int vallen);

///
/// Adds a new header line to the list
///
/// param this HDRLineRef A ref for the list being added to
/// param label char* A pointer to a c_string buffer with the key or label. '0' terminated
/// param value char* pointer to a c_string buffer holding the value of the header line '0' terminated
///
/// The content of the char* are copied into the header line so the caller is free
/// to deal with the two char* buffers as they wish.
///
///
void HDRList_add_cstr(HDRListRef this, char* label, char* value);

#endif
