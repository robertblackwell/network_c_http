
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/http/header_list.h>
#include <rbl/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define TYPE HttpMessage
#define HttpMessage_TAG "DemMSG"
#include <rbl/check_tag.h>
#undef TYPE
#define DEMO_MESSAGE_DECLARE_TAG RBL_DECLARE_TAG(HttpMessage_TAG)
#define DEMO_MESSAGE_CHECK_TAG(p) RBL_CHECK_TAG(HttpMessage_TAG, p)
#define DEMO_MESSAGE_SET_TAG(p) RBL_SET_TAG(HttpMessage_TAG, p)

struct HttpMessage_s
{
    RBL_DECLARE_TAG;
    bool            is_request;
    BufferChainRef  body;
    RBL_DECLARE_END_TAG;
};

HttpMessageRef http_message_new ()
{
    HttpMessageRef mref = (HttpMessageRef) eg_alloc(sizeof(HttpMessage));
    DEMO_MESSAGE_SET_TAG(mref)
    RBL_SET_END_TAG(HttpMessage_TAG, mref);
    if(mref == NULL) goto error_label_1;
    mref->body = NULL;
    mref->body = BufferChain_new();
    return mref;

    error_label_1:
        return NULL;
}
static char get_body_first_char(HttpMessageRef msg_ref)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    char first_byte = (char)(*(char*)(IOBuffer_data(iob)));
    return first_byte;
}
static char set_body_first_char(HttpMessageRef msg_ref, char first_byte)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    (*(char*)(IOBuffer_data(iob))) = first_byte;
}
/**
 * @brief Create a new request message instance
 * @return HttpMessageRef
 */
HttpMessageRef http_message_new_request()
{
    HttpMessageRef mref = http_message_new();
    RBL_CHECK_TAG(HttpMessage_TAG, mref);
    RBL_CHECK_END_TAG(HttpMessage_TAG, mref);
    return mref;
}
HttpMessageRef http_message_new_response()
{

    HttpMessageRef mref = http_message_new();
    RBL_CHECK_TAG(HttpMessage_TAG, mref);
    RBL_CHECK_END_TAG(HttpMessage_TAG, mref);
    return mref;
}
void http_message_free(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);

    BufferChain_free(((this)->body));
    this->body = NULL;
    eg_free(this);
}
IOBufferRef http_message_serialize(HttpMessageRef mref)
{
    RBL_CHECK_TAG(HttpMessage_TAG, mref);
    RBL_CHECK_END_TAG(HttpMessage_TAG, mref);
    BufferChainRef bc = BufferChain_new();
    char* start_str = "\02";
    char* end_str = "\03";
    BufferChain_append(bc, (void*)start_str, 1);
    BufferChain_append_bufferchain(bc, mref->body);
    BufferChain_append(bc, (void*) end_str, 1);
    IOBufferRef result = BufferChain_compact(bc);
    BufferChain_free((bc));
    return result;
}
bool http_message_get_is_request(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);
}
void http_message_set_is_request(HttpMessageRef this, bool yn)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);
    return;
    if(yn)
        set_body_first_char(this, 'Q');
    else
        set_body_first_char(this, 'R');
}
void http_message_set_content_length(HttpMessageRef this, int length)
{
    assert(false);
//    char buf[10];
//    assert(length >= 0);
//    int r = sprintf(buf, "%d", length);
//    HdrListRef hdrlist_ref = this->headers;
//    KVPairRef kvp = HdrList_find(hdrlist_ref, "Content-length");
//    if(kvp != NULL) {
//        KVPair_set_value(kvp, buf, strlen(buf));
//    } else {
//        HdrList_add_cstr(hdrlist_ref, "Content-length", buf);
//    }
}
void http_message_set_lrc(HttpMessageRef this, char lrc)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);
}
BufferChainRef http_message_get_body(HttpMessageRef this)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);
    return this->body;
}
void http_message_set_body(HttpMessageRef this, BufferChainRef bc)
{
    RBL_CHECK_TAG(HttpMessage_TAG, this);
    RBL_CHECK_END_TAG(HttpMessage_TAG, this);
    if(this->body != NULL) {
        RBL_LOG_FMT("http_message_set_body existing body being ignored bc: %p  this->body: %p", bc, this->body);
        BufferChain_free((this->body));
    }
    this->body = bc;
}
/**@}*/