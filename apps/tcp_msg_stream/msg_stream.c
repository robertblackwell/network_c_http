#include "msg_stream.h"

MsgStreamRef msg_stream_new(RunloopRef rl, int fd)
{
    MsgStreamRef ms = malloc(sizeof(MsgStream));
    msg_stream_init(ms, rl, fd);
    return ms;
}
void msg_stream_reader_init(MsgStreamRef ms);
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
    msg_stream_reader_init(msg_stream);
}
void msg_stream_free(MsgStreamRef msg_stream_ref)
{   
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    tcp_stream_free(msg_stream_ref->tcp_stream_ref);
    generic_msg_parser_free(msg_stream_ref->msg_parser_ref);
    List_safe_free(msg_stream_ref->input_message_list, free);
    if(msg_stream_ref->input_buffer) IOBuffer_free(msg_stream_ref->input_buffer);
    if(msg_stream_ref->output_buffer) IOBuffer_free(msg_stream_ref->output_buffer);
    free(msg_stream_ref);
}
