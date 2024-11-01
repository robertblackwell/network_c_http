
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/http/header_list.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define TYPE DemoMessage
#define DemoMessage_TAG "DemMSG"
#include <rbl/check_tag.h>
#undef TYPE
#define DEMO_MESSAGE_DECLARE_TAG DECLARE_TAG(DemoMessage_TAG)
#define DEMO_MESSAGE_CHECK_TAG(p) CHECK_TAG(DemoMessage_TAG, p)
#define DEMO_MESSAGE_SET_TAG(p) SET_TAG(DemoMessage_TAG, p)

struct DemoMessage_s
{
    DECLARE_TAG;
    bool            opcode; //can be either 'REQ' or 'RESP'
    uint8_t         lrc;
    BufferChainRef  body;
};

DemoMessageRef demo_message_new ()
{
    DemoMessageRef mref = (DemoMessageRef) eg_alloc(sizeof(DemoMessage));
    DEMO_MESSAGE_SET_TAG(mref)
    if(mref == NULL) goto error_label_1;
    mref->body = NULL;
    mref->opcode = false;
    mref->body = BufferChain_new();
    return mref;

    error_label_1:
        return NULL;
}
/**
 * @brief Create a new request message instance
 * @return DemoMessageRef
 */
DemoMessageRef demo_message_new_request()
{
    DemoMessageRef mref = demo_message_new();
    return mref;
}
DemoMessageRef demo_message_new_response()
{

    DemoMessageRef mref = demo_message_new();
    return mref;
}
void demo_message_free(DemoMessageRef this)
{
    DEMO_MESSAGE_CHECK_TAG(this)

    BufferChain_dispose(&((this)->body));
    eg_free(this);
}
void demo_message_dispose(DemoMessageRef* this_p)
{
    DEMO_MESSAGE_CHECK_TAG(*this_p)
    DemoMessageRef this = *this_p;
    DEMO_MESSAGE_CHECK_TAG(this)
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
    DEMO_MESSAGE_CHECK_TAG(mref)
    char sbuf[3];
    sbuf[0] = 0x01;
    sbuf[1] = (mref->opcode) ? 'Q': 'R';
    sbuf[2] = 0x02;
    BufferChainRef bc = BufferChain_new();
    IOBufferRef b = IOBuffer_from_buf(sbuf, 3);
    BufferChain_append_IOBuffer(bc, b);
    BufferChain_append_bufferchain(bc, mref->body);
    char* end_str = "\03L\x04";
    BufferChain_append(bc, (void*) end_str, 3);
    IOBufferRef result = BufferChain_compact(bc);
    BufferChain_dispose(&(bc));
    IOBuffer_dispose(&(b));
    return result;
}
bool demo_message_get_is_request(DemoMessageRef this)
{
    DEMO_MESSAGE_CHECK_TAG(this);
    return this->opcode;
}
void demo_message_set_is_request(DemoMessageRef this, bool yn)
{
    DEMO_MESSAGE_CHECK_TAG(this);
    this->opcode = yn;
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
    DEMO_MESSAGE_CHECK_TAG(this);
    this->lrc = lrc;
}
BufferChainRef demo_message_get_body(DemoMessageRef this)
{
    DEMO_MESSAGE_CHECK_TAG(this);
    return this->body;
}
void demo_message_set_body(DemoMessageRef this, BufferChainRef bc)
{
    DEMO_MESSAGE_CHECK_TAG(this);
    if(this->body != NULL) {
        printf("demomessage_set_body existing body being ignored bc: %p  this->body: %p\n", bc, this->body);
        BufferChain_dispose(&(this->body));
//        BufferChain_free(this->body);
//        this->body = NULL;
    }
    this->body = bc;
}
/**@}*/