#include "echo_msg.h"
#include <rbl/check_tag.h>
#include <common/iobuffer.h>
struct EchoMsg_s {
    RBL_DECLARE_TAG;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
};

// struct EchoMsgParser_s {
//     RBL_DECLARE_TAG;
//     char line_buffer[1000];
//     int line_buffer_length;
//     int line_buffer_max;
//     EchoMsgRef msg_ref;
//     IOBufferRef iob;
//     RBL_DECLARE_END_TAG;
// };

EchoMsgRef echo_msg_new()
{
    EchoMsgRef msg_ref = malloc(sizeof(EchoMsg));
    echo_msg_init(msg_ref);
    return msg_ref;
}
void echo_msg_init(EchoMsgRef msg_ref)
{
    RBL_SET_TAG(EchoMsg_TAG, msg_ref)
    RBL_SET_END_TAG(EchoMsg_TAG, msg_ref)
    msg_ref->iob = IOBuffer_new();
}
void echo_msg_deinit(EchoMsgRef m)
{
    RBL_CHECK_TAG(EchoMsg_TAG, m)
    RBL_CHECK_END_TAG(EchoMsg_TAG, m)
    if(m->iob) IOBuffer_destroy(m->iob);
}
void echo_msg_free(EchoMsgRef m)
{
    RBL_CHECK_TAG(EchoMsg_TAG, m)
    RBL_CHECK_END_TAG(EchoMsg_TAG, m)
    if(m->iob) IOBuffer_free(m->iob);
    free(m);
}
IOBufferRef echo_msg_serialize(EchoMsgRef msg)
{
    char* p = (char*)IOBuffer_cstr(msg->iob);
    IOBufferRef iob = IOBuffer_from_cstring(p);
    // now frame the message
    IOBuffer_commit_push_back(iob, '\n');

    return iob;
}
IOBufferRef echo_msg_get_content(EchoMsgRef msg_ref)
{
    assert(msg_ref != NULL);
    return msg_ref->iob;
}
void echo_msg_set_content(EchoMsgRef mr, IOBufferRef iob)
{
    // assert(mr->iob == NULL);
    if (mr->iob != NULL ) {
        IOBuffer_free(mr->iob);
        mr->iob = NULL;
    }
    mr->iob = iob;
}
