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
void* generic_message_new(){
    return message_new();
}
void generic_message_free(void* msg_ref){
    message_free(msg_ref);
}
IOBufferRef generic_message_serialize(void* msg_ref){
    return message_serialize(msg_ref);
}
IOBufferRef generic_message_get_content(void* msg_ref){
    return message_get_content(msg_ref);
}
void generic_message_set_content(void* msg_ref, IOBufferRef iob){
    message_set_content(msg_ref, iob);
};
void* generic_parser_new(){
    return msg_parser_new();
}
void generic_parser_free(void* msg_ref){
    msg_parser_free(msg_ref);
}
void generic_parser_consume(
    void* parser_ref,
    IOBufferRef new_data,
    void(cb)(void* user_ctx, void* new_msg_ref, int error),
    void* arg){
    MsgParserCallback* mpcb = (MsgParserCallback*)cb;
    return msg_parser_consume(parser_ref, new_data, mpcb, arg);
}
static IMessage imessage_var;
IMessageRef msg_get_message_interface()
{
    IMessageRef mr = &imessage_var;
    mr->message_new = &generic_message_new;
    mr->message_free = &generic_message_free;
    mr->serialize = &generic_message_serialize;
    mr->get_content = &generic_message_get_content;
    mr->set_content = &generic_message_set_content;

    mr->parser_new = &generic_parser_new;
    mr->parser_free = &generic_parser_free;
    mr->parser_consume = &generic_parser_consume;
    return mr;
}
