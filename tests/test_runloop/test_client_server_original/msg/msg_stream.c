#include "msg_stream.h"

MsgStreamRef msg_stream_new(RunloopRef rl, int fd)
{
    MsgStreamRef ms = malloc(sizeof(MsgStream));
    msg_stream_init(ms, rl, fd);
    return ms;
}
void msg_stream_init(MsgStreamRef msg_stream, RunloopRef rl, int fd)
{
    RBL_SET_TAG(MsgStream_TAG, msg_stream)
    RBL_SET_END_TAG(MsgStream_TAG, msg_stream)
    msg_stream->tcp_stream_ref = tcp_stream_new(rl, fd);
    msg_stream->read_cb = NULL;
    msg_stream->read_cb_arg = NULL;
    msg_stream->write_cb = NULL;
    msg_stream->write_cb_arg = NULL;
    msg_stream->input_buffer = NULL;
    msg_stream->output_buffer = NULL;
    msg_stream->input_message_list = List_new();
    msg_stream->msg_parser_ref = msg_parser_new();
}
void msg_stream_deinit(MsgStreamRef msg_stream_ref)
{
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    tcp_stream_deinit(msg_stream_ref->tcp_stream_ref);
    msg_parser_deinit(msg_stream_ref->msg_parser_ref);
    if(msg_stream_ref->input_buffer) IOBuffer_destroy(msg_stream_ref->input_buffer);
    if(msg_stream_ref->output_buffer) IOBuffer_destroy(msg_stream_ref->output_buffer);
}
void msg_stream_free(MsgStreamRef msg_stream_ref)
{   
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    tcp_stream_free(msg_stream_ref->tcp_stream_ref);
    msg_parser_free(msg_stream_ref->msg_parser_ref);
    List_safe_free(msg_stream_ref->input_message_list, free);
    if(msg_stream_ref->input_buffer) IOBuffer_free(msg_stream_ref->input_buffer);
    if(msg_stream_ref->output_buffer) IOBuffer_free(msg_stream_ref->output_buffer);
    free(msg_stream_ref);
}

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
IOBufferRef message_get_content(MessageRef msg)
{
    assert(msg->iob);
    return msg->iob;
}
void message_set_content(MessageRef msg, IOBufferRef iob)
{
    if((msg->iob != NULL) && (msg->iob != iob))  {
        IOBuffer_free(msg->iob);
    }
    msg->iob = iob;
}
