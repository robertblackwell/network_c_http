#ifndef c_ceg_http_header_h
#define c_ceg_http_header_h
#include <rbl/check_tag.h>
#include <src/common/cbuffer.h>
/**
 * @addtogroup group_hdrlist
 * @{
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// WARNING The content between these block comments is generated code and will be over written at the next build
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <src/common/list.h>
typedef struct HttpHeaders_s HttpHeaders;
typedef size_t HttpHeaderIterator;

typedef struct HttpHeaderString_s
{
    const char* value_ptr;
    const size_t* size;
} HttpHeaderString;

typedef struct HttpHeaderEntry_s
{
    HttpHeaderString* key;
    HttpHeaderString* value;
} HttpHeaderEntry;

typedef struct LineOffset_s
{
    int16_t offset;
    int16_t length;
} LineOffset;

/**
 * Header lines are character arrays (not cstrings) of the form "key_array:value_array\r\n"
 * The key_array has only non-blank alpha characters
 * The value_string is free form but no control characters.
 */
typedef char* HttpHeaderLinePtr;

/**
 * A Http message header consists of a first_line and then an array of HttpHeaderLines
 * The header lines are stored in the lines_buffer field.
 * The start of each line is stored in lines_offset as a int16_t value so that the first
 * character of the n-th corresponding line is at address (lines_buffer+lines_offset[n])
 *
 * The number of non blank lines is given by the size field.
 * The first blank line is at index 'size' and is the string "\r\n"
 */
struct HttpHeaders_s
{
    RBL_DECLARE_TAG;
    size_t size;
    size_t*     line_offsets_start; //This is an array of header lines of the form "key:value\r\n"
    char*       line_offsets_raw; //
    char*       line_offsets_end;
    char*       line_offsets_tag;
    char*       line_offsets_end_tag;
    /**
     * This is an array of header lines of the form "key:value\r\n"
     */
    bool        waiting_for_value; // if true have added key but waiting for value next_offset is for value
    size_t      next_offset;  // index into lines_buffer for the next addition
    char*       lines_buffer_next; // points to the addr of the next location to accept a character
    char*       current_line_ptr; // points to the start of the line that is currently being built
    char*       lines_buffer_start; //This is an array of header lines of the form "key:value\r\n"
    char*       lines_buffer_raw; //
    char*       lines_buffer_end;
    char*       lines_buffer_tag;
    char*       lines_buffer_end_tag;
    RBL_DECLARE_END_TAG
};

HttpHeaders*  http_header_new(size_t nbr_lines, size_t sizeof_lines_buffer);
void http_header_free(HttpHeaders* header);
void http_header_init(HttpHeaders* header, size_t nbr_lines, size_t sizeof_lines_buffer);
void http_header_deinit(HttpHeaders* header);

size_t      http_header_size(HttpHeaders* header);
void http_header_set_pair(HttpHeaders* hdrs, const char* key, int klen, const char* value, int vlen);
/**
 * The next two functions are dangerous as they set only a part of a header line
 * If they are not called correctly (without other header manipulation between them)
 * the header structure will be damaged.
 */
void http_header_set_key(HttpHeaders* header, const char* key, int len);
void http_header_set_value(HttpHeaders* header, const char* value, int len);

void http_header_append_key(HttpHeaders* header, const char* key, int klen);
void http_header_begin_value(HttpHeaders* header, const char* value, int vlen);
void http_header_append_value(HttpHeaders* header, const char* value, int vlen);
void http_header_end_line(HttpHeaders* header);
#if 0
KVPairRef  http_header_first(HttpHeader* header);
KVPairRef  http_header_last(HttpHeader* header) ;
KVPairRef  http_header_remove_first(HttpHeader* header);
KVPairRef  http_header_remove_last(HttpHeader* header);
KVPairRef  http_header_itr_unpack(HttpHeader* header, HttpHeaderIterator iter);
HttpHeaderIterator http_header_iterator(HttpHeader* header);
HttpHeaderIterator http_header_itr_next  (HttpHeader* header, HttpHeaderIterator iter);
void        http_header_itr_remove(HttpHeader* header, HttpHeaderIterator* iter_addr);
void http_header_add_back(HttpHeader* header, const char* key, const char* value);

size_t      http_header_size(HttpHeader* header);
HttpHeaderEntry http_header_at(HttpHeader* header, size_t index);

const char* http_header_key_at(HttpHeader* header, size_t index);
const char* http_header_value_at(HttpHeader* header, size_t index);

const char* http_header_find(HttpHeader* header, const char* key);
/**
 * Convenience function so that a list of headers can be filled in with code like:
 *
 * HdrListRef hdrlist = HdrList_from_array({
 *  {"Content-type", "plain/html"},
 *  {"Connection", "close"},
 *  {"Content-length", "102"},
 *  {NULL, NULL}
 *  });
 *  All the values in the array must be either sttring constants or local stack variables
 *  as this function will not deallocate any of these strings
 *
 *
 * @param raw
 * @return
 */
HdrListRef HdrList_from_array(const char* raw[][2]);


///
/// Create a new KVPair instance from key and value and add
/// that KVPair to the HdrList.
///
/// The content of key and value are copied into the new KVPair instance
/// and hence ownership of key and value remain with the caller
///
/// param this HdrListRef
/// param key CbufferRef
/// param CbufferRef
/// return void
///
void HdrList_add(HdrListRef this, const CbufferRef key, const CbufferRef value);

/**
 * Add multiple headers lines to a header list in one call.
 * \param this  The HdrList to add the header lines
 * \param pairs CStrPair[] An array of CStrPair terminated by null
 */
void HdrList_add_many(HdrListRef this, CStrPair* pairs[]);
void HdrList_add_arr(HdrListRef this, const char* ar[][2]);
///
/// Find a KVPair in a HdrList by key/label value
///
/// param hlref HdrListRef
/// param key char*
/// return KVPairRef or NULL
/// NULL on not found
/// NOTE: If found the KVPairRef returns is still owned by the HdrList
/// do not call KVPair_dispose() on the returned value
///
KVPairRef HdrList_find(const HdrListRef hlref, const char* key);

///
/// Remove a KVPair from the HdrList by key/label
///
/// param hlref HdrListRef
/// param key char*
///
void HdrList_remove(HdrListRef hlref, const char* key);

/// Serialize a header list into a CbufferRef
/// param this HdrListRef
/// return A serialized version of the header list as a Cbuffer.
/// NOTE: ownership of the Cbuffer is transfered to the caller
CbufferRef HdrList_serialize(const HdrListRef this);

///
/// Adds a new header line to the list
///
/// param this HDRLineRef A ref for the list being added to
/// param key CbufferRef holding the key or label for the header line.
/// param value CbufferRef holding the value for the header line
///
/// The content of the CbufferRef are copied into the header line so the caller is free
/// to deal with the two CbufferRef as they wish.
///
///
void HdrList_add_cbuf(HdrListRef this, const CbufferRef key, const CbufferRef value);

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
void HdrList_add_line(HdrListRef this, const char* label, int lablen, const char* value, int vallen);

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
void HdrList_add_cstr(HdrListRef this, const char* label, const char* value);
#endif
/** @} */

#endif
