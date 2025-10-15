
#include <src/demo_protocol/demo_message.h>
#include <src/common/alloc.h>
#include <src/http/header_list.h>
#include <rbl/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define TYPE Message
#define Message_TAG "DemMSG"
#include <rbl/check_tag.h>
#undef TYPE
#define DEMO_MESSAGE_DECLARE_TAG RBL_DECLARE_TAG(Message_TAG)
#define DEMO_MESSAGE_CHECK_TAG(p) RBL_CHECK_TAG(Message_TAG, p)
#define DEMO_MESSAGE_SET_TAG(p) RBL_SET_TAG(Message_TAG, p)
#define DEMO_MESSAGE_SET_END_TAG(p) RBL_SET_END_TAG(Message_TAG, p)

struct Message_s
{
    RBL_DECLARE_TAG;
    bool            is_request;
    BufferChainRef  body;
    RBL_DECLARE_END_TAG;
};

MessageRef message_new ()
{
    MessageRef mref = (MessageRef) malloc(sizeof(Message));
    DEMO_MESSAGE_SET_TAG(mref)
    RBL_SET_END_TAG(Message_TAG, mref);
    assert(mref != NULL);
    mref->body = NULL;
    mref->body = BufferChain_new();
    return mref;
}
static char get_body_first_char(MessageRef msg_ref)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    char first_byte = (char)(*(char*)(IOBuffer_data(iob)));
    return first_byte;
}
static void set_body_first_char(MessageRef msg_ref, char first_byte)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    (*(char*)(IOBuffer_data(iob))) = first_byte;
}
/**
 * @brief Create a new request message instance
 * @return MessageRef
 */
MessageRef message_new_request()
{
    MessageRef mref = message_new();
    RBL_CHECK_TAG(Message_TAG, mref);
    RBL_CHECK_END_TAG(Message_TAG, mref);
    return mref;
}
MessageRef message_new_response()
{
    MessageRef mref = message_new();
    RBL_CHECK_TAG(Message_TAG, mref);
    RBL_CHECK_END_TAG(Message_TAG, mref);
    return mref;
}
void message_free(MessageRef this)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
    BufferChain_free(((this)->body));
    this->body = NULL;
    free(this);
}
void message_anonymous_free(void* p)
{
    message_free((MessageRef)p);
}
IOBufferRef message_serialize(MessageRef mref)
{
    RBL_CHECK_TAG(Message_TAG, mref);
    RBL_CHECK_END_TAG(Message_TAG, mref);
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
bool message_get_is_request(MessageRef this)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
    return get_body_first_char(this) == 'R';
}
void message_set_is_request(MessageRef this, bool yn)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
    return;
}
void message_set_content_length(MessageRef this, int length)
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
void message_set_lrc(MessageRef this, char lrc)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
}
BufferChainRef message_get_body(MessageRef this)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
    return this->body;
}
void message_set_body(MessageRef this, BufferChainRef bc)
{
    RBL_CHECK_TAG(Message_TAG, this);
    RBL_CHECK_END_TAG(Message_TAG, this);
    if(this->body != NULL) {
        RBL_LOG_FMT("demomessage_set_body existing body being ignored bc: %p  this->body: %p", bc, this->body);
        BufferChain_free((this->body));
    }
    this->body = bc;
}
/**@}*/