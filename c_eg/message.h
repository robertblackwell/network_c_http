#ifndef c_eg_message_h
#define c_eg_message_h
#include <stdbool.h>
#include <http-parser/http_parser.h>
#include <c_eg/buffer/buffer_chain.h>

#include <c_eg/headerline_list.h>

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

HttpStatus Message_get_method(MessageRef mref);
void Message_set_method(MessageRef mref, HttpMethod method);

HttpMinorVersion Message_get_minor_version(MessageRef mref);
void Message_set_minor_version(MessageRef this, HttpMinorVersion mv);
void Message_set_version(MessageRef this, int maj, int minor);

HDRListRef Message_headers(MessageRef this);

void Message_add_header(MessageRef mref, char* labptr, int lablen, char* valptr, int vallen);
HeaderIter Message_get_header(MessageRef mref, const char* labptr);
bool Message_get_is_request(MessageRef this);
void Message_set_is_request(MessageRef this, bool yn);
void Message_move_target(MessageRef this, CBufferRef target);
char* Message_get_target(MessageRef this);

void Message_move_reason(MessageRef this, CBufferRef reason);
char* Message_get_reason(MessageRef this);
/**
 *
 * @param mref MessageRef
 * @param iter HeaderListIter
 * @return a pointer to a c_string that is not owned by caller
 */
const char* Message_header_iter_deref(MessageRef mref, HeaderIter iter);

BufferChainRef Message_get_body(MessageRef mref);
void Message_set_body(MessageRef mref, BufferChainRef bodyp);

#endif