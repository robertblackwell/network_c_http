#ifndef c_ceg_hdrlist_h
#define c_ceg_hdrlist_h
#include <c_eg/kvpair.h>
#include <c_eg/buffer/contig_buffer.h>


__LIST_INCLUDE_H__

///
/// Create a new KVPair instance from key and value and add
/// that KVPair to the HdrList.
///
/// The content of key and value are copied into the new KVPair instance
/// and hence ownership of key and value remain with the caller
///
/// \param this HdrList*
/// \param key CBufferRef
/// \param CBufferRef
/// \return void
///
void HdrList_add(HdrList* this, CBufferRef key, CBufferRef value);

///
/// Find a KVPair in a HdrList by key/label value
///
/// \param hlref HdrList*
/// \param key   char*
/// \return    KVPair* or NULL
///            NULL on not found
///            NOTE: If found the KVPair* returns is still owned by the HdrList
///            do not call KVPair_free() on the returned value
///
KVPair* HdrList_find(HdrList* hlref, char* key);

///
/// Remove a KVPair from the HdrList by key/label
///
/// \param hlref HdrList*
/// \param key   char*
///
void HdrList_remove(HdrList* hlref, char* key);

/// Serialize a header list into a CBufferRef
/// \param this HdrList*
/// \return A serialized version of the header list as a CBuffer.
///         NOTE: ownership of the CBuffer is transfered to the caller
CBufferRef HdrList_serialize(HdrList* this);

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
void HdrList_add_cbuf(HdrList* this, CBufferRef key, CBufferRef value);

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
void HdrList_add_line(HdrList* this, char* label, int lablen, char* value, int vallen);

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
void HdrList_add_cstr(HdrList* this, char* label, char* value);

#endif