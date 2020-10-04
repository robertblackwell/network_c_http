#ifndef c_eg_message_h
#define c_eg_message_h
#include <stdbool.h>
#include <http-parser/http_parser.h>
#include <c_eg/buffer/buffer_chain.h>

#include <c_eg/hdrlist.h>

struct Message_s;
typedef struct Message_s Message;

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

Message* Message_new();
Message* Message_new_request();
Message* Message_new_response();

void Message_free(Message** p);
void Message_dealloc(void* m);

bool Message_is_request(Message* mref);

HttpStatus Message_get_status(Message* mref);
void Message_set_status(Message* mref, HttpStatus status);

HttpMethod Message_get_method(Message* mref);
void Message_set_method(Message* mref, HttpMethod method);

HttpMinorVersion Message_get_minor_version(Message* mref);
void Message_set_minor_version(Message* this, HttpMinorVersion mv);
void Message_set_version(Message* this, int maj, int minor);

HdrList* Message_headers(Message* this);

void Message_add_header(Message* mref, char* labptr, int lablen, char* valptr, int vallen);

///
/// Returns the header list (of type HdrList*) of a message.
///
/// NOTE: the memory for the returned value remains owned by the Message*
///
/// \param this Message*
/// \return HdrList*. The returned value is a reference ownership stays with the Message*.
///
HeaderIter Message_get_header(Message* mref, const char* labptr);

///
/// Returns true if the Message* points at a request false otherwise.
///
/// \param this
/// \return bool
///
bool Message_get_is_request(Message* this);

///
/// Sets a Message internal is_request flag to the given value/
///
/// \param this Message*
/// \param yn   bool
///
void Message_set_is_request(Message* this, bool yn);

///
/// Sets the content of Message* target property to the content of Cbuffer target
/// argument.
///
/// NOTE: does this using move sematics so that the argument is reset to an empty buffer
/// after the call.
///
/// NOTE: the caller retains ownership of the target argument
///
/// \param this   Message*
/// \param target Cbuffer
///
void Message_move_target(Message* this, Cbuffer* target);

///
/// Returns a char*/c_string  pointer to the target string in a request message.
///
/// NOTE: the memory for the returned value remains owned by the Message*
///
/// \param this Message*
/// \return c_string pointer. The returned value is a reference, ownership stays with the Message*.
///
Cbuffer* Message_get_target(Message* this);
/// set target
void Message_set_target(Message* this, char* targ);

///
/// Sets the content of Message* reason property to the content of Cbuffer target
/// argument.
///
/// NOTE: does this using move sematics so that the argument is reset to an empty buffer
/// after the call.
///
/// NOTE: the caller retains ownership of the target argument
///
/// \param this   Message*
/// \param reason Cbuffer
///
void Message_move_reason(Message* this, Cbuffer* reason);

///
/// Returns a char*/c_string  pointer to the reason string in a response message.
///
/// NOTE: the memory for the returned value remains owned by the Message*
///
/// \param this Message*
/// \return c_string pointer. The returned value is a reference ownership stays with the Message*.
///
char* Message_get_reason(Message* this);

///
/// Returns the value property of a header line, pointed at by iter, from the headers of Message*.
///
/// This is basically a derefence operation
///
/// \param mref Message*
/// \param iter HeaderListIter Must not be NULL
/// \return a pointer to a c_string that is not owned by caller
///
///
const char* Message_header_iter_deref(Message* mref, HeaderIter iter);

BufferChain* Message_get_body(Message* mref);
void Message_set_body(Message* mref, BufferChain* bodyp);

Cbuffer* Message_serialize(Message* this);

#endif