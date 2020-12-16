#ifndef c_ceg_hdrlist_h
#define c_ceg_hdrlist_h
#include <c_http/dsl/kvpair.h>
#include <c_http/api/cbuffer.h>


__LIST_INCLUDE_H__

///
/// Create a new KVPair instance from key and value and add
/// that KVPair to the HdrList.
///
/// The content of key and value are copied into the new KVPair instance
/// and hence ownership of key and value remain with the caller
///
/// \param this HdrListRef
/// \param key CbufferRef
/// \param CbufferRef
/// \return void
///
void HdrList_add(HdrListRef this, CbufferRef key, CbufferRef value);

///
/// Find a KVPair in a HdrList by key/label value
///
/// \param hlref HdrListRef
/// \param key   char*
/// \return    KVPairRef or NULL
///            NULL on not found
///            NOTE: If found the KVPairRef returns is still owned by the HdrList
///            do not call KVPair_dispose() on the returned value
///
KVPairRef HdrList_find(HdrListRef hlref, char* key);

///
/// Remove a KVPair from the HdrList by key/label
///
/// \param hlref HdrListRef
/// \param key   char*
///
void HdrList_remove(HdrListRef hlref, char* key);

/// Serialize a header list into a CbufferRef
/// \param this HdrListRef
/// \return A serialized version of the header list as a Cbuffer.
///         NOTE: ownership of the Cbuffer is transfered to the caller
CbufferRef HdrList_serialize(HdrListRef this);

///
/// Adds a new header line to the list
///
/// param this   HDRLineRef  A ref for the list being added to
/// param key    CbufferRef  holding the key or label for the header line.
/// param value  CbufferRef  holding the value for the header line
///
/// The content of the CbufferRef are copied into the header line so the caller is free
/// to deal with the two CbufferRef as they wish.
///
///
void HdrList_add_cbuf(HdrListRef this, CbufferRef key, CbufferRef value);

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
void HdrList_add_line(HdrListRef this, char* label, int lablen, char* value, int vallen);

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
void HdrList_add_cstr(HdrListRef this, char* label, char* value);

#endif