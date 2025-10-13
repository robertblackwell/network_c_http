#include "message.h"
#include "message_internal.h"
#include <rbl/check_tag.h>
#include <common/iobuffer.h>
// typedef struct Message_s {
//     RBL_DECLARE_TAG;
//     IOBufferRef iob;
//     RBL_DECLARE_END_TAG;
// } Message, * MessageRef;
//
// typedef void(MsgReadCallback)(void* arg, MessageRef msg, int error);
// typedef void(MsgWriteCallback)(void* arg, int error);
//
// typedef struct MsgParser_s {
//     RBL_DECLARE_TAG;
//     char line_buffer[1000];
//     int line_buffer_length;
//     int line_buffer_max;
//     MessageRef msg_ref;
//     IOBufferRef iob;
//     RBL_DECLARE_END_TAG;
// } MsgParser, *MsgParserRef;

MessageRef message_new()
{
    MessageRef msg_ref = malloc(sizeof(Message));
    message_init(msg_ref);
    return msg_ref;
}
void message_init(MessageRef msg_ref)
{
    RBL_SET_TAG(Message_TAG, msg_ref)
    RBL_SET_END_TAG(Message_TAG, msg_ref)
    msg_ref->iob = IOBuffer_new();
}
void message_deinit(MessageRef m)
{
    RBL_CHECK_TAG(Message_TAG, m)
    RBL_CHECK_END_TAG(Message_TAG, m)
    if(m->iob) IOBuffer_destroy(m->iob);
}
void message_free(MessageRef m)
{
    RBL_CHECK_TAG(Message_TAG, m)
    RBL_CHECK_END_TAG(Message_TAG, m)
    if(m->iob) IOBuffer_free(m->iob);
    free(m);
}
IOBufferRef message_serialize(MessageRef msg)
{
    char* p = (char*)IOBuffer_cstr(msg->iob);
    IOBufferRef iob = IOBuffer_from_cstring(p);
    // now frame the message
    IOBuffer_commit_push_back(iob, '\n');

    return iob;
}
IOBufferRef message_get_content(MessageRef msg_ref)
{
    assert(msg_ref != NULL);
    return msg_ref->iob;
}
void message_set_content(MessageRef mr, IOBufferRef iob)
{
    // assert(mr->iob == NULL);
    if (mr->iob != NULL ) {
        IOBuffer_free(mr->iob);
        mr->iob = NULL;
    }
    mr->iob = iob;
}