#include "newline_msg.h"


NewlineMsgRef newline_msg_new()
{
    NewlineMsgRef msg_ref = malloc(sizeof(NewlineMsg));
    newline_msg_init(msg_ref);
    return msg_ref;
}
void newline_msg_init(NewlineMsgRef msg_ref)
{
    RBL_SET_TAG(NewlineMsg_TAG, msg_ref)
    RBL_SET_END_TAG(NewlineMsg_TAG, msg_ref)
    msg_ref->iob = IOBuffer_new();
}
void newline_msg_deinit(NewlineMsgRef m)
{
    RBL_CHECK_TAG(NewlineMsg_TAG, m)
    RBL_CHECK_END_TAG(NewlineMsg_TAG, m)
    if(m->iob) IOBuffer_destroy(m->iob);
}
void newline_msg_free(NewlineMsgRef m)
{
    RBL_CHECK_TAG(NewlineMsg_TAG, m)
    RBL_CHECK_END_TAG(NewlineMsg_TAG, m)
    if(m->iob) IOBuffer_free(m->iob);
    free(m);
}
IOBufferRef newline_msg_serialize(NewlineMsgRef msg)
{
    char* p = (char*)IOBuffer_cstr(msg->iob);
    IOBufferRef iob = IOBuffer_from_cstring(p);
    // now frame the message
    IOBuffer_commit_push_back(iob, '\n');

    return iob;
}
IOBufferRef newline_msg_get_content(NewlineMsgRef msg)
{
    assert(msg->iob);
    return msg->iob;
}
void newline_msg_set_content(NewlineMsgRef msg, IOBufferRef iob)
{
    if((msg->iob != NULL) && (msg->iob != iob))  {
        IOBuffer_free(msg->iob);
    }
    msg->iob = iob;
}
