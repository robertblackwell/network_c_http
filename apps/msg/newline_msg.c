#include "newline_msg.h"
#include <rbl/check_tag.h>
#include <common/iobuffer.h>
struct NewLineMsg_s {
    RBL_DECLARE_TAG;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
};

// struct NewLineMsgParser_s {
//     RBL_DECLARE_TAG;
//     char line_buffer[1000];
//     int line_buffer_length;
//     int line_buffer_max;
//     NewLineMsgRef msg_ref;
//     IOBufferRef iob;
//     RBL_DECLARE_END_TAG;
// };

NewLineMsgRef newline_msg_new()
{
    NewLineMsgRef msg_ref = malloc(sizeof(NewLineMsg));
    newline_msg_init(msg_ref);
    return msg_ref;
}
void newline_msg_init(NewLineMsgRef msg_ref)
{
    RBL_SET_TAG(NewLineMsg_TAG, msg_ref)
    RBL_SET_END_TAG(NewLineMsg_TAG, msg_ref)
    msg_ref->iob = IOBuffer_new();
}
void newline_msg_deinit(NewLineMsgRef m)
{
    RBL_CHECK_TAG(NewLineMsg_TAG, m)
    RBL_CHECK_END_TAG(NewLineMsg_TAG, m)
    if(m->iob) IOBuffer_destroy(m->iob);
}
void newline_msg_free(NewLineMsgRef m)
{
    RBL_CHECK_TAG(NewLineMsg_TAG, m)
    RBL_CHECK_END_TAG(NewLineMsg_TAG, m)
    if(m->iob) IOBuffer_free(m->iob);
    free(m);
}
IOBufferRef newline_msg_serialize(NewLineMsgRef msg)
{
    char* p = (char*)IOBuffer_cstr(msg->iob);
    IOBufferRef iob = IOBuffer_from_cstring(p);
    // now frame the message
    IOBuffer_commit_push_back(iob, '\n');

    return iob;
}
IOBufferRef newline_msg_get_content(NewLineMsgRef msg_ref)
{
    assert(msg_ref != NULL);
    return msg_ref->iob;
}
void newline_msg_set_content(NewLineMsgRef mr, IOBufferRef iob)
{
    // assert(mr->iob == NULL);
    if (mr->iob != NULL ) {
        IOBuffer_free(mr->iob);
        mr->iob = NULL;
    }
    mr->iob = iob;
}
