
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/http/header_list.h>
#include <rbl/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define TYPE DemoMessage
#define DemoMessage_TAG "DemMSG"
#include <rbl/check_tag.h>
#undef TYPE
#define DEMO_MESSAGE_DECLARE_TAG RBL_DECLARE_TAG(DemoMessage_TAG)
#define DEMO_MESSAGE_CHECK_TAG(p) RBL_CHECK_TAG(DemoMessage_TAG, p)
#define DEMO_MESSAGE_SET_TAG(p) RBL_SET_TAG(DemoMessage_TAG, p)

struct DemoMessage_s
{
    RBL_DECLARE_TAG;
    bool            is_request;
    BufferChainRef  body;
    RBL_DECLARE_END_TAG;
};

DemoMessageRef demo_message_new ()
{
    DemoMessageRef mref = (DemoMessageRef) eg_alloc(sizeof(DemoMessage));
    DEMO_MESSAGE_SET_TAG(mref)
    RBL_SET_END_TAG(DemoMessage_TAG, mref);
    if(mref == NULL) goto error_label_1;
    mref->body = NULL;
    mref->body = BufferChain_new();
    return mref;

    error_label_1:
        return NULL;
}
static char get_body_first_char(DemoMessageRef msg_ref)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    char first_byte = (char)(*(char*)(IOBuffer_data(iob)));
    return first_byte;
}
static char set_body_first_char(DemoMessageRef msg_ref, char first_byte)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    (*(char*)(IOBuffer_data(iob))) = first_byte;
}
/**
 * @brief Create a new request message instance
 * @return DemoMessageRef
 */
DemoMessageRef demo_message_new_request()
{
    DemoMessageRef mref = demo_message_new();
    RBL_CHECK_TAG(DemoMessage_TAG, mref);
    RBL_CHECK_END_TAG(DemoMessage_TAG, mref);
    return mref;
}
DemoMessageRef demo_message_new_response()
{

    DemoMessageRef mref = demo_message_new();
    RBL_CHECK_TAG(DemoMessage_TAG, mref);
    RBL_CHECK_END_TAG(DemoMessage_TAG, mref);
    return mref;
}
void demo_message_free(DemoMessageRef this)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);

    BufferChain_dispose(&((this)->body));
    eg_free(this);
}
void demo_message_dispose(DemoMessageRef* this_p)
{
    DEMO_MESSAGE_CHECK_TAG(*this_p)
    DemoMessageRef this = *this_p;
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
    BufferChain_dispose(&((*this_p)->body));
    eg_free(*this_p);
    *this_p = NULL;
}
void demo_message_dispose_anonymous(void* p)
{
    demo_message_dispose((DemoMessageRef*)&p);
}
IOBufferRef demo_message_serialize(DemoMessageRef mref)
{
    RBL_CHECK_TAG(DemoMessage_TAG, mref);
    RBL_CHECK_END_TAG(DemoMessage_TAG, mref);
    BufferChainRef bc = BufferChain_new();
    char* start_str = "\02";
    char* end_str = "\03";
    BufferChain_append(bc, (void*)start_str, 1);
    BufferChain_append_bufferchain(bc, mref->body);
    BufferChain_append(bc, (void*) end_str, 1);
    IOBufferRef result = BufferChain_compact(bc);
    BufferChain_dispose(&(bc));
    return result;
}
bool demo_message_get_is_request(DemoMessageRef this)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
}
void demo_message_set_is_request(DemoMessageRef this, bool yn)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
    return;
    if(yn)
        set_body_first_char(this, 'Q');
    else
        set_body_first_char(this, 'R');
}
void demo_message_set_content_length(DemoMessageRef this, int length)
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
void demo_message_set_lrc(DemoMessageRef this, char lrc)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
}
BufferChainRef demo_message_get_body(DemoMessageRef this)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
    return this->body;
}
void demo_message_set_body(DemoMessageRef this, BufferChainRef bc)
{
    RBL_CHECK_TAG(DemoMessage_TAG, this);
    RBL_CHECK_END_TAG(DemoMessage_TAG, this);
    if(this->body != NULL) {
        RBL_LOG_FMT("demomessage_set_body existing body being ignored bc: %p  this->body: %p", bc, this->body);
        BufferChain_dispose(&(this->body));
    }
    this->body = bc;
}
/**@}*/