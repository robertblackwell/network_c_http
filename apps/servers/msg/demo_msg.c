
#include "demo_msg.h"
#include <src/common/alloc.h>
#include <src/http/header_list.h>
#include <rbl/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define TYPE DemoMsg
#define DemoMsg_TAG "DemMSG"
#include <rbl/check_tag.h>
#undef TYPE
#define DEMO_MESSAGE_DECLARE_TAG RBL_DECLARE_TAG(DemoMsg_TAG)
#define DEMO_MESSAGE_CHECK_TAG(p) RBL_CHECK_TAG(DemoMsg_TAG, p)
#define DEMO_MESSAGE_SET_TAG(p) RBL_SET_TAG(DemoMsg_TAG, p)
#define DEMO_MESSAGE_SET_END_TAG(p) RBL_SET_END_TAG(DemoMsg_TAG, p)

struct DemoMsg_s
{
    RBL_DECLARE_TAG;
    bool            is_request;
    BufferChainRef  body;
    RBL_DECLARE_END_TAG;
};

DemoMsgRef demo_msg_new ()
{
    DemoMsgRef mref = (DemoMsgRef) malloc(sizeof(DemoMsg));
    DEMO_MESSAGE_SET_TAG(mref)
    RBL_SET_END_TAG(DemoMsg_TAG, mref);
    assert(mref != NULL);
    mref->body = NULL;
    mref->body = BufferChain_new();
    return mref;
}
static char get_body_first_char(DemoMsgRef msg_ref)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    char first_byte = (char)(*(char*)(IOBuffer_data(iob)));
    return first_byte;
}
static void set_body_first_char(DemoMsgRef msg_ref, char first_byte)
{
    assert(BufferChain_size(msg_ref->body));
    IOBufferRef iob = BufferChain_front(msg_ref->body);
    (*(char*)(IOBuffer_data(iob))) = first_byte;
}
/**
 * @brief Create a new request demo_msg instance
 * @return DemoMsgRef
 */
DemoMsgRef demo_msg_new_request()
{
    DemoMsgRef mref = demo_msg_new();
    RBL_CHECK_TAG(DemoMsg_TAG, mref);
    RBL_CHECK_END_TAG(DemoMsg_TAG, mref);
    return mref;
}
DemoMsgRef demo_msg_new_response()
{
    DemoMsgRef mref = demo_msg_new();
    RBL_CHECK_TAG(DemoMsg_TAG, mref);
    RBL_CHECK_END_TAG(DemoMsg_TAG, mref);
    return mref;
}
void demo_msg_free(DemoMsgRef this)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
    BufferChain_free(((this)->body));
    this->body = NULL;
    free(this);
}
void demo_msg_anonymous_free(void* p)
{
    demo_msg_free((DemoMsgRef)p);
}
IOBufferRef demo_msg_serialize(DemoMsgRef mref)
{
    RBL_CHECK_TAG(DemoMsg_TAG, mref);
    RBL_CHECK_END_TAG(DemoMsg_TAG, mref);
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
bool demo_msg_get_is_request(DemoMsgRef this)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
    return get_body_first_char(this) == 'R';
}
void demo_msg_set_is_request(DemoMsgRef this, bool yn)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
    return;
}
void demo_msg_set_content_length(DemoMsgRef this, int length)
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
void demo_msg_set_lrc(DemoMsgRef this, char lrc)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
}
BufferChainRef demo_msg_get_body(DemoMsgRef this)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
    return this->body;
}
IOBufferRef demo_msg_get_content(DemoMsgRef msg_ref)
{
    return BufferChain_compact(msg_ref->body);
}
void demo_msg_set_content(DemoMsgRef msg_ref, IOBufferRef content)
{
    if (msg_ref->body) {
        BufferChain_free(msg_ref->body);
    }
    msg_ref->body = BufferChain_new();
    BufferChain_append_IOBuffer(msg_ref->body, content);
}
void demo_msg_set_body(DemoMsgRef this, BufferChainRef bc)
{
    RBL_CHECK_TAG(DemoMsg_TAG, this);
    RBL_CHECK_END_TAG(DemoMsg_TAG, this);
    if(this->body != NULL) {
        RBL_LOG_FMT("demodemo_msg_set_body existing body being ignored bc: %p  this->body: %p", bc, this->body);
        BufferChain_free((this->body));
    }
    this->body = bc;
}
/**@}*/