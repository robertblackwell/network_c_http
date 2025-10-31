
#include "stx_msg.h"
#include <src/common/alloc.h>
#include <src/http/header_list.h>
#include <rbl/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define StxMsg_TAG "STXMSG"
#include <rbl/check_tag.h>

struct StxMsg_s
{
    RBL_DECLARE_TAG;
    bool            is_request;
//    BufferChainRef  body;
    IOBufferRef     body;
    RBL_DECLARE_END_TAG;
};

StxMsgRef stx_msg_new ()
{
    StxMsgRef mref = (StxMsgRef) eg_alloc(sizeof(StxMsg));
    RBL_SET_TAG(StxMsg_TAG, mref)
    RBL_SET_END_TAG(StxMsg_TAG, mref);
    assert(mref != NULL);
    mref->body = NULL;
    mref->body = IOBuffer_new();
    return mref;
}
void stx_msg_free(StxMsgRef this)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);

    IOBuffer_free(((this)->body));
    this->body = NULL;
    eg_free(this);
}

static char get_body_first_char(StxMsgRef msg_ref)
{
    assert(IOBuffer_data_len(msg_ref->body));
    char first_byte = (char)(*(char*)(IOBuffer_data(msg_ref->body)));
    return first_byte;
}
static char set_body_first_char(StxMsgRef msg_ref, char first_byte)
{
    assert(IOBuffer_data_len(msg_ref->body));
    (*(char*)(IOBuffer_data(msg_ref->body))) = first_byte;
}
/**
 * @brief Create a new request message instance
 * @return StxMsgRef
 */
StxMsgRef stx_msg_new_request()
{
    StxMsgRef mref = stx_msg_new();
    RBL_CHECK_TAG(StxMsg_TAG, mref);
    RBL_CHECK_END_TAG(StxMsg_TAG, mref);
    return mref;
}
StxMsgRef stx_msg_new_response()
{

    StxMsgRef mref = stx_msg_new();
    RBL_CHECK_TAG(StxMsg_TAG, mref);
    RBL_CHECK_END_TAG(StxMsg_TAG, mref);
    return mref;
}
void stx_msg_anonymous_free(void* p)
{
    stx_msg_free((StxMsgRef)p);
}
IOBufferRef stx_msg_serialize(StxMsgRef mref)
{
    RBL_CHECK_TAG(StxMsg_TAG, mref);
    RBL_CHECK_END_TAG(StxMsg_TAG, mref);
    IOBufferRef iob = IOBuffer_new_with_capacity (IOBuffer_data_len(mref->body) + 64);
    IOBuffer_sprintf(iob, "\02%s\03", IOBuffer_cstr(mref->body));
    return iob;
}
IOBufferRef stx_msg_get_content(StxMsgRef msgref)
{
    RBL_CHECK_TAG(StxMsg_TAG, msgref);
    RBL_CHECK_END_TAG(StxMsg_TAG, msgref);
    return msgref->body;
}
void stx_msg_set_content(StxMsgRef msgref, IOBufferRef iob)
{
    if(iob != msgref->body) {
        IOBufferRef tmp = msgref->body;
        msgref->body = iob;
        IOBuffer_free(tmp);
    }
}
bool stx_msg_get_is_request(StxMsgRef this)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);
}
void stx_msg_set_is_request(StxMsgRef this, bool yn)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);
    return;
    if(yn)
        set_body_first_char(this, 'Q');
    else
        set_body_first_char(this, 'R');
}
void stx_msg_set_content_length(StxMsgRef this, int length)
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
void stx_msg_set_lrc(StxMsgRef this, char lrc)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);
}
IOBufferRef stx_msg_get_body(StxMsgRef this)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);
    return this->body;
}
void stx_msg_set_body(StxMsgRef this, IOBufferRef iob)
{
    RBL_CHECK_TAG(StxMsg_TAG, this);
    RBL_CHECK_END_TAG(StxMsg_TAG, this);
    if(this->body != iob) {
        RBL_LOG_FMT("demomessage_set_body existing body being ignored bc: %p  this->body: %p", iob, this->body);
        IOBuffer_free((this->body));
        this->body = iob;
    }
}
/**@}*/