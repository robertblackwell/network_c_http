#ifndef H_c_http_http_message_internal_H
#define  H_c_http_http_message_internal_H
#include "http_message.h"
#include <src/test_helpers/message_private.h>
// #include <src/http/header_list.h>
#include <stdbool.h>

struct HttpMessage_s
{
    RBL_DECLARE_TAG;
    BufferChainRef body;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    HttpStatus status_code;
    HttpMethod method;
    void* allocator;
    CbufferRef reason;
    CbufferRef target;
    HeaderListPtr headers;
};

void http_message_target_append(HttpMessage* msg, char* at, size_t length);
void http_message_reason_append(HttpMessage* msg, char* at, size_t length);

void http_message_add_empty_headerline(HttpMessage* msg);
HeaderLine* http_message_headers_last(HttpMessage* msg);
void http_message_last_header_line_append_to_key(HttpMessage* msg, void* buf, size_t length);
void http_message_last_header_line_append_to_value(HttpMessage* msg, void* buf, size_t length);

#endif
/**@}*/