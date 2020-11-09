#ifndef c_eg_message_h
#define c_eg_message_h
#include <stdbool.h>
#include <http-parser/http_parser.h>
#include <c_eg/buffer/buffer_chain.h>

#include <c_eg/hdrlist.h>

struct Message_s;
typedef struct Message_s Message, *MessageRef;

// from http-parser
typedef enum http_status HttpStatus;
typedef enum http_method HttpMethod;
typedef enum http_errno HttpErrno;
enum HttpMajorVersion {major_version1=1};
enum HttpMinorVersion {minor_verson0=0, minor_version1=1};

#define HttpStatusString(status) http_status_str(status)
#define HttpMethodString(method) http_method_str(method)
#define HttpErrnoName(er) http_errno_name(er)
#define HttpErrnoDescription(er) http_errno_description(er)

#define HEADER_HOST "HOST"
#define HEADER_CONNECTION "CONNECTION"
#define HEADER_PROXYCONNECTION "PROXY-CONNECTION"
#define HEADER_CONTENT_LENGTH "CONTENT-LENGTH"
#define HEADER_TRANSFERENCODING "TRANSFER-ENCODING"
#define HEADER_CONTENT_TYPE "CONTENT-TYPE"
#define HEADER_ECHO_ID "C-EG-ECHO-ID"


// an iterator for pointing at a header line. NULL means does not reference a header line
typedef void* HeaderIter;

typedef int HttpMinorVersion;

MessageRef Message_new();
MessageRef Message_new_request();
MessageRef Message_new_response();

void Message_free(MessageRef* p);
void Message_dealloc(void* m);

bool Message_is_request(MessageRef mref);

HttpStatus Message_get_status(MessageRef mref);
void Message_set_status(MessageRef mref, HttpStatus status);

HttpMethod Message_get_method(MessageRef mref);
void Message_set_method(MessageRef mref, HttpMethod method);

HttpMinorVersion Message_get_minor_version(MessageRef mref);
void Message_set_minor_version(MessageRef this, HttpMinorVersion mv);
void Message_set_version(MessageRef this, int maj, int minor);

HdrListRef Message_headers(MessageRef this);

void Message_add_header(MessageRef mref, char* labptr, int lablen, char* valptr, int vallen);

///
/// Returns the header list (of type HdrListRef) of a message.
///
/// NOTE: the memory for the returned value remains owned by the MessageRef
///
/// \param this MessageRef
/// \return HdrListRef. The returned value is a reference ownership stays with the MessageRef.
///
HeaderIter Message_get_header(MessageRef mref, const char* labptr);

///
/// Returns true if the MessageRef points at a request false otherwise.
///
/// \param this
/// \return bool
///
bool Message_get_is_request(MessageRef this);

///
/// Sets a Message internal is_request flag to the given value/
///
/// \param this MessageRef
/// \param yn   bool
///
void Message_set_is_request(MessageRef this, bool yn);

///
/// Sets the content of MessageRef target property to the content of Cbuffer target
/// argument.
///
/// NOTE: does this using move sematics so that the argument is reset to an empty buffer
/// after the call.
///
/// NOTE: the caller retains ownership of the target argument
///
/// \param this   MessageRef
/// \param target Cbuffer
///
void Message_move_target(MessageRef this, CbufferRef target);

///
/// Returns a char*/c_string  pointer to the target string in a request message.
///
/// NOTE: the memory for the returned value remains owned by the MessageRef
///
/// \param this MessageRef
/// \return c_string pointer. The returned value is a reference, ownership stays with the MessageRef.
///
CbufferRef Message_get_target(MessageRef this);
/// set target
void Message_set_target(MessageRef this, char* targ);

///
/// Sets the content of MessageRef reason property to the content of Cbuffer target
/// argument.
///
/// NOTE: does this using move sematics so that the argument is reset to an empty buffer
/// after the call.
///
/// NOTE: the caller retains ownership of the target argument
///
/// \param this   MessageRef
/// \param reason Cbuffer
///
void Message_move_reason(MessageRef this, CbufferRef reason);

///
/// Returns a char*/c_string  pointer to the reason string in a response message.
///
/// NOTE: the memory for the returned value remains owned by the MessageRef
///
/// \param this MessageRef
/// \return c_string pointer. The returned value is a reference ownership stays with the MessageRef.
///
char* Message_get_reason(MessageRef this);

///
/// Returns the value property of a header line, pointed at by iter, from the headers of MessageRef.
///
/// This is basically a derefence operation
///
/// \param mref MessageRef
/// \param iter HeaderListIter Must not be NULL
/// \return a pointer to a c_string that is not owned by caller
///
///
const char* Message_header_iter_deref(MessageRef mref, HeaderIter iter);

BufferChainRef Message_get_body(MessageRef mref);
void Message_set_body(MessageRef mref, BufferChainRef bodyp);

CbufferRef Message_serialize(MessageRef this);

#endif