
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/test_helpers/message_private.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/http/header_list.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/**
 * @addtogroup group_message
 * @{
 */


#include <rbl/check_tag.h>



struct HttpMessage_s
{
    RBL_DECLARE_TAG;
    BufferChainRef body;
    HdrListRef headers;
    int major_vers;
    HttpMinorVersion minor_vers;
    bool is_request;
    HttpStatus status_code;
    CbufferRef reason;
    HttpMethod method;
    CbufferRef target;
};

HttpMessageRef http_message_new ()
{
    HttpMessageRef mref = (HttpMessageRef) eg_alloc(sizeof(HttpMessage));
    if(mref == NULL) goto error_label_1;
    RBL_SET_TAG(HttpMessage_TAG, mref)
    mref->body = NULL;
    mref->headers = HdrList_new();
    if(mref->headers == NULL) goto error_label_2;
    mref->minor_vers = minor_version1;
    mref->major_vers = major_version1;
    mref->target = Cbuffer_new();
    mref->reason = Cbuffer_new();
    return mref;

    error_label_2:
    http_message_free(mref);
    error_label_1:
        return NULL;
}
/**
 * @brief Create a new request message instance
 * @return HttpMessageRef
 */
HttpMessageRef http_message_new_request()
{
    HttpMessageRef mref = http_message_new();
    if(mref != NULL) {
        mref->is_request = true;
//        mref->target = Cbuffer_new();
        return mref;
    }
    return NULL;
}
HttpMessageRef http_message_new_response()
{

    HttpMessageRef mref = http_message_new();
    if(mref != NULL) {
        mref->is_request = false;
//        mref->reason = Cbuffer_new();
        return mref;
    }
    return NULL;
}
void http_message_free(HttpMessageRef p)
{
    RBL_CHECK_TAG(HttpMessage_TAG, p)
    HdrList_safe_free(p->headers);
    Cbuffer_free(p->target);
    Cbuffer_free(p->reason);
    eg_free(p);
}
void http_message_anonymous_free(void* p)
{
    http_message_free(p);
}
HttpMessageRef MessageResponse(HttpStatus status, void* body)
{
    HttpMessageRef mref = http_message_new();
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
    HdrListRef hdrs = this->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPairRef item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        BufferChain_append_cstr(bc_result, s);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
    BufferChain_append_cstr(bc_result, "\r\n");
    if((this->body != NULL) && (BufferChain_size(this->body) != 0)) {
        IOBufferRef iob_body = BufferChain_compact(this->body);
        BufferChain_add_back(bc_result, iob_body);
    }
    IOBufferRef result = BufferChain_compact(bc_result);
    BufferChain_free(bc_result);
    return result;
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
    HdrListRef hdrs = this->headers;
    ListIterator iter = HdrList_iterator(hdrs);
    while(iter != NULL) {
        KVPairRef item = HdrList_itr_unpack(hdrs, iter);
        char* s;
        int len = asprintf(&s,"%s: %s\r\n", KVPair_label(item), KVPair_value(item));
        BufferChain_append_cstr(bc_result, s);
        ListIterator next = HdrList_itr_next(hdrs, iter);
        iter = next;
        free(s);
    }
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
    HdrListRef hdrlist = http_message_get_headerlist(mref);
    HdrList_add_cstr(hdrlist, label, value);
}
void http_message_add_header_cbuf(HttpMessageRef this, CbufferRef key, CbufferRef value)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    HdrList_add_cbuf(http_message_get_headerlist(this), key, value);
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
//void Message_move_target(HttpMessageRef this, CbufferRef target)
//{
//    if(this->target == NULL) {
//        this->target = Cbuffer_new();
//    }
//    Cbuffer_move(this->target, target);
//}
void http_message_set_target(HttpMessageRef this, const char* target_cstr)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    assert((this->target != NULL) && (Cbuffer_size(this->target) == 0));
    Cbuffer_append_cstr(this->target, (const char*)target_cstr);
}
CbufferRef http_message_get_target_cbuffer(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return Cbuffer_from_cstring(Cbuffer_cstr(this->target));
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
//void Message_move_reason(HttpMessageRef this, CbufferRef reason)
//{
//    if(this->reason == NULL)
//        this->reason = Cbuffer_new();
//    Cbuffer_move(this->reason, reason);
//}
const char* http_message_get_reason(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    (const char*)Cbuffer_cstr(this->reason);
}
CbufferRef http_message_get_reason_cbuffer(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return Cbuffer_from_cstring(Cbuffer_cstr(this->reason));
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
    HdrListRef hdrlist_ref = this->headers;
    KVPairRef kvp = HdrList_find(hdrlist_ref, "Content-length");
    if(kvp != NULL) {
        KVPair_set_value(kvp, buf, strlen(buf));
    } else {
        HdrList_add_cstr(hdrlist_ref, "Content-length", buf);
    }
}

// headers
HdrListRef http_message_get_headerlist(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this)
    return this->headers;
}
const char* http_message_get_header_value(HttpMessageRef mref, const char* labptr)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref)
    KVPairRef kvp = HdrList_find(http_message_get_headerlist(mref), labptr);
    if(kvp == NULL) {
        return NULL;
    }
    return KVPair_value(kvp);
}
int HttpMessage_cmp_header(HttpMessageRef msgref, const char* key, const char* test_value)
{
    RBL_CHECK_TAG(HttpMessage_TAG, msgref)
    KVPairRef kvp = HdrList_find(http_message_get_headerlist(msgref), key);
    if(kvp == NULL) {
        return -1;
    }
    char* v = KVPair_value(kvp);
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
    HdrList_add_arr(mref->headers, ar);
}
/**@}*/