
#include "http_message.h"
#include "http_message_internal.h"
#include <src/test_helpers/message_private.h>
#include <src/http/header_list.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <rbl/check_tag.h>
#include <common/alloc.h>
/**
 * @addtogroup group_message
 * @{
 */

void http_message_init (HttpMessageRef mref, Allocator* allocator_ptr)
{
    RBL_SET_TAG(HttpMessage_TAG, mref)
    assert(allocator_ptr != NULL);
    mref->body = NULL;
    mref->allocator = allocator_ptr;
    mref->minor_vers = minor_version1;
    mref->major_vers = major_version1;
    mref->target = Cbuffer_new(allocator_ptr);
    mref->reason = Cbuffer_new(allocator_ptr);
    mref->headers = header_list_new(allocator_ptr);
}
HttpMessageRef http_message_new_with_allocator(Allocator* allocator)
{
    HttpMessageRef mref = (HttpMessageRef) allocator_alloc(allocator, sizeof(HttpMessage));
    assert(mref != NULL);
    http_message_init(mref, allocator);
    return mref;
}
HttpMessageRef http_message_new(Allocator* allocator)
{
    if(allocator == NULL) {
        allocator = (Allocator*) default_allocator_create();
    }
    return http_message_new_with_allocator(allocator);
}
/**
 * @brief Create a new request message instance
 * @return HttpMessageRef
 */
HttpMessageRef http_message_new_request(Allocator* allocator)
{
    HttpMessageRef mref = http_message_new(allocator);
    if(mref != NULL) {
        mref->is_request = true;
        return mref;
    }
    return NULL;
}
HttpMessageRef http_message_new_response(Allocator* allocator)
{

    HttpMessageRef mref = http_message_new(allocator);
    if(mref != NULL) {
        mref->is_request = false;
        return mref;
    }
    return NULL;
}
void http_message_free(HttpMessageRef p)
{
    RBL_CHECK_TAG(HttpMessage_TAG, p)
    Allocator* a = p->allocator;
    header_list_free(p->headers);
    Cbuffer_free(p->target);
    Cbuffer_free(p->reason);
    allocator_dealloc(a, p);
}
void http_message_anonymous_free(void* p)
{
    http_message_free(p);
}
HttpMessageRef MessageResponse(HttpStatus status, void* body, Allocator* allocator)
{
    HttpMessageRef mref = http_message_new(allocator);
    if(mref == NULL) goto error_1;
    mref->is_request = false;
    mref->status_code = status;
    mref->body = body;
    if(mref->target != NULL) Cbuffer_free(mref->target);
    if(mref->reason != NULL) Cbuffer_free(mref->reason);
    return mref;
    error_1:
        return NULL;
}
IOBufferRef http_message_serialize(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    char* first_line;
    int first_line_len;
    if(this->is_request) {
        const char* meth = llhttp_method_name(this->method);
        first_line_len = asprintf(&first_line, "%s %s HTTP/%d.%d\r\n", meth, (char*)Cbuffer_data(this->target), this->major_vers, this->minor_vers);
    } else {
        first_line_len = asprintf(&first_line, "HTTP/%d.%d  %d %s\r\n", this->major_vers, this->minor_vers, this->status_code, (char*)Cbuffer_data(this->reason));
    }
    IOBufferRef ioresult = IOBuffer_from_cstring(first_line);
    free(first_line);
    HeaderListPtr hdrs = this->headers;
    CbufferRef hdr_serialize = header_list_serialize(hdrs);
    IOBuffer_append_cstr(ioresult, (char*)Cbuffer_cstr(hdr_serialize));
    Cbuffer_free(hdr_serialize);
    IOBuffer_append_cstr(ioresult, "\r\n");
    if((this->body != NULL) && (BufferChain_size(this->body) != 0)) {
        IOBufferRef iob_body = BufferChain_compact(this->body);
        IOBuffer_append_cstr(ioresult, IOBuffer_cstr(iob_body));
    }
    IOBufferRef result2 = ioresult;
    return result2;
}
IOBufferRef http_message_dump(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    BufferChainRef bc_result = BufferChain_new();
    char* first_line;
    int first_line_len;
    if(this->is_request) {
        const char* meth = llhttp_method_name(this->method);
        first_line_len = asprintf(&first_line, "%s %s HTTP/%d.%d\r\n", meth, (char*)Cbuffer_data(this->target), this->major_vers, this->minor_vers);
    } else {
        first_line_len = asprintf(&first_line, "HTTP/%d.%d  %d %s\r\n", this->major_vers, this->minor_vers, this->status_code, (char*)Cbuffer_data(this->reason));
    }
    BufferChain_append_cstr(bc_result, first_line);
    free(first_line);
    HeaderListPtr hdrs = this->headers;
    CbufferRef hdump = header_list_serialize(hdrs);
    BufferChain_append_cstr(bc_result, (char*)Cbuffer_cstr(hdump));
    BufferChain_append_cstr(bc_result, "\r\n");
    BufferChain_append_cstr(bc_result, "body begin ===========================================================\r\n");
    if((this->body != NULL) && (BufferChain_size(this->body) != 0)) {
        IOBufferRef iob_body = BufferChain_compact(this->body);
        BufferChain_add_back(bc_result, iob_body);
    }
    BufferChain_append_cstr(bc_result, "body end ===========================================================\r\n");
    IOBufferRef result = BufferChain_compact(bc_result);
    BufferChain_free(bc_result);
    return result;
}
void http_message_add_header_cstring(HttpMessageRef mref, const char* label, const char* value)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    HeaderListPtr hdrlist = http_message_get_headerlist(mref);
    header_list_add_cstr(hdrlist, label, value);
}
void http_message_add_header_cbuf(HttpMessageRef this, CbufferRef key, CbufferRef value)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    header_list_add_cbuf(http_message_get_headerlist(this), key, value);
}
HttpStatus http_message_get_status(HttpMessageRef mref)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    return mref->status_code;
}
void http_message_set_status(HttpMessageRef mref, HttpStatus status)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    mref->status_code = status;
}
bool http_message_get_is_request(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return this->is_request;
}
void http_message_set_is_request(HttpMessageRef this, bool yn)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    this->is_request = yn;
}
HttpMinorVersion http_message_get_minor_version(HttpMessageRef mref)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    return mref->minor_vers;
}

void http_message_set_minor_version(HttpMessageRef this, HttpMinorVersion mv)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    this->minor_vers = mv;
}
void http_message_set_version(HttpMessageRef this, int maj, int minor)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    this->minor_vers = minor;
}
void http_message_set_method(HttpMessageRef mref, HttpMethod method)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    mref->method = method;
}
HttpMethod http_message_get_method(HttpMessageRef mref)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    return mref->method;
}

// target
const char* http_message_get_target(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return (const char*)Cbuffer_cstr(this->target);
}
void http_message_set_target(HttpMessageRef this, const char* target_cstr)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    assert((this->target != NULL) && (Cbuffer_size(this->target) == 0));
    Cbuffer_append_cstr(this->target, (const char*)target_cstr);
}
CbufferRef http_message_get_target_cbuffer(HttpMessageRef this)
{
    assert(0);
    // RBL_CHECK_TAG(HttpMessage_TAG, this)
    // return Cbuffer_from_cstring(Cbuffer_cstr(this->target));
}
void http_message_set_target_cbuffer(HttpMessageRef this, CbufferRef target)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    assert((this->target != NULL) && (Cbuffer_size(this->target) == 0));
    Cbuffer_append_cstr(this->target, (const char*)Cbuffer_cstr(target));

}
// reason
void http_message_set_reason(HttpMessageRef this, const char* reason_cstr)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    assert((this->reason != NULL) && (Cbuffer_size(this->reason) == 0));
    Cbuffer_append_cstr(this->reason, (const char*)reason_cstr);
}
const char* http_message_get_reason(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return (const char*)Cbuffer_cstr(this->reason);
}
CbufferRef http_message_get_reason_cbuffer(HttpMessageRef this)
{
    assert(0);
    // RBL_CHECK_TAG(HttpMessage_TAG, this)
    // return Cbuffer_from_cstring(Cbuffer_cstr(this->reason));
}
void http_message_set_reason_cbuffer(HttpMessageRef this, CbufferRef reason)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    assert((this->reason != NULL) && (Cbuffer_size(this->reason) == 0));
    Cbuffer_append_cstr(this->reason, (const char*)Cbuffer_cstr(reason));
}
int Message_get_content_length(HttpMessageRef this)
{
    assert(false);
}
void http_message_set_content_length(HttpMessageRef this, int length)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    char buf[100];
    assert(length >= 0);
    int r = sprintf(buf, "%d", length);
    HeaderListPtr hdrlist_ref = this->headers;
    HeaderLinePtr hline = header_list_find(hdrlist_ref, "Content-length");
    if(hline != NULL) {
        header_line_set_value(hline, Cbuffer_from_cstring(buf, this->allocator));
    } else {
        header_list_add_cstr(hdrlist_ref, "Content-length", buf);
    }
}

// headers
void http_message_set_headers(HttpMessageRef msg, HeaderListPtr hlist)
{
    RBL_CHECK_TAG(HttpMessage_TAG, msg)
    assert(hlist != NULL);
    msg->headers = hlist;
}

HeaderListPtr http_message_get_headerlist(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return this->headers;
}
const char* http_message_get_header_value(HttpMessageRef mref, const char* labptr)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    HeaderLinePtr line = header_list_find(http_message_get_headerlist(mref), labptr);
    if(line == NULL) {
        return NULL;
    }
    return Cbuffer_cstr(line->value);
}
int HttpMessage_cmp_header(HttpMessageRef msgref, const char* key, const char* test_value)
{
    RBL_CHECK_TAG(HttpMessage_TAG, msgref)
    HeaderLinePtr line = header_list_find(http_message_get_headerlist(msgref), key);
    if(line == NULL) {
        return -1;
    }
    const char* v = Cbuffer_cstr(line->value);
    if(strlen(v) != strlen(test_value)) {
        return 0;
    }
    for(int i = 0; i < strlen(v); i++) {
        if(toupper(v[i]) != toupper(test_value[i])) {
            return 0;
        }
    }
    return 1;
}

BufferChainRef http_message_get_body(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return this->body;
}
void http_message_set_body(HttpMessageRef mref, BufferChainRef bodyp)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    mref->body = bodyp;
}
void http_message_set_headers_arr(HttpMessageRef mref, const char* ar[][2])
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    header_list_add_arr(mref->headers, ar);
}
IOBufferRef http_message_get_content(HttpMessageRef mref)
{
    return NULL;
}
void http_message_set_content(HttpMessageRef mref, IOBufferRef iob)
{
    
}
void http_message_target_append(HttpMessage* msg, char* at, size_t length)
{
    Cbuffer_append(msg->target, at, length);
}
void http_message_reason_append(HttpMessage* msg, char* at, size_t length)
{
    Cbuffer_append(msg->reason, at, length);
}
void http_message_add_empty_headerline(HttpMessage* msg)
{
    Allocator* a = msg->allocator;
    header_list_add_back(msg->headers, header_line_new(Cbuffer_new(a), Cbuffer_new(a), a));
}
HeaderLine* http_message_headers_last(HttpMessage* msg)
{
    return header_list_last(msg->headers);
}
void http_message_last_header_line_append_to_key(HttpMessage* msg, void* buf, size_t length)
{
    header_line_append_key(header_list_last(msg->headers), buf, (int)length);
}
void http_message_last_header_line_append_to_value(HttpMessage* msg, void* buf, size_t length)
{
    header_line_append_value(header_list_last(msg->headers), buf, (int)length);
}

/**@}*/