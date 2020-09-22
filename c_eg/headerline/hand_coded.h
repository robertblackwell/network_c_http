#ifndef c_ceg_headerline_list_h
#define c_ceg_headerline_list_h
#include <c_eg/header_line.h>
#include <c_eg/buffer/contig_buffer.h>


__LIST_INCLUDE_H__

///
/// Create a new HeaderLine instance from key and value and add
/// that HeaderLine to the HDRList.
///
/// The content of key and value are copied into the new HeaderLine instance
/// and hence ownership of key and value remain with the caller
///
/// \param this HDRListRef
/// \param key CBufferRef
/// \param CBufferRef
/// \return void
///
void HDRList_add(HDRListRef this, CBufferRef key, CBufferRef value);

///
/// Find a HeaderLine in a HDRList by key/label value
///
/// \param hlref HDRListRef
/// \param key   char*
/// \return    HeaderLineRef or NULL
///            NULL on not found
///            NOTE: If found the HeaderLineRef returns is still owned by the HDRList
///            do not call HeaderLine_free() on the returned value
///
HeaderLineRef HDRList_find(HDRListRef hlref, char* key);

///
/// Remove a HeaderLine from the HDRList by key/label
///
/// \param hlref HDRListRef
/// \param key   char*
///
void HDRList_remove(HDRListRef hlref, char* key);

/// Serialize a header list into a CBufferRef
/// \param this HDRListRef
/// \return A serialized version of the header list as a CBuffer.
///         NOTE: ownership of the CBuffer is transfered to the caller
CBufferRef HDRList_serialize(HDRListRef this);

///
/// Adds a new header line to the list
///
/// param this   HDRLineRef  A ref for the list being added to
/// param key    CBufferRef  holding the key or label for the header line.
/// param value  CBufferRef  holding the value for the header line
///
/// The content of the CBufferRef are copied into the header line so the caller is free
/// to deal with the two CBufferRef as they wish.
///
///
void HDRList_add_cbuf(HDRListRef this, CBufferRef key, CBufferRef value);

///
/// Adds a new header line to the list
///
/// param this   HDRLineRef  A ref for the list being added to
/// param label    char* A pointer to a string buffer with the key or label
/// param lablen   int the length of the key or label, no assumptions about zero terminated
///param value    char* pointer to a string buffer holding the value of the header line
/// param vallen   int the length of the value, no assumptions about zero terminated
///
/// The content of the char* are copied into the header line so the caller is free
/// to deal with the two char* buffers as they wish.
///
////
void HDRList_add_line(HDRListRef this, char* label, int lablen, char* value, int vallen);

///
/// Adds a new header line to the list
///
/// param this   HDRLineRef  A ref for the list being added to
/// param label    char* A pointer to a c_string buffer with the key or label. '\0' terminated
/// param value    char* pointer to a c_string buffer holding the value of the header line '\0' terminated
///
/// The content of the char* are copied into the header line so the caller is free
/// to deal with the two char* buffers as they wish.
///
///
void HDRList_add_cstr(HDRListRef this, char* label, char* value);

#endif