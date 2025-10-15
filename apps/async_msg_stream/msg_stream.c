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
    msg_stream->msg_parser_ref = MSG_PARSER_NEW();
}
#if 0
void msg_stream_deinit(MsgStreamRef msg_stream_ref)
{
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    tcp_stream_deinit(msg_stream_ref->tcp_stream_ref);
    MSG_PARSER_DEINIT(msg_stream_ref->msg_parser_ref);
    if(msg_stream_ref->input_buffer) IOBuffer_destroy(msg_stream_ref->input_buffer);
    if(msg_stream_ref->output_buffer) IOBuffer_destroy(msg_stream_ref->output_buffer);
}
#endif
void msg_stream_free(MsgStreamRef msg_stream_ref)
{   
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    tcp_stream_free(msg_stream_ref->tcp_stream_ref);
    MSG_PARSER_FREE(msg_stream_ref->msg_parser_ref);
    if(msg_stream_ref->input_buffer) IOBuffer_free(msg_stream_ref->input_buffer);
    if(msg_stream_ref->output_buffer) IOBuffer_free(msg_stream_ref->output_buffer);
}
RunloopRef msg_stream_get_runloop(MsgStreamRef msg_stream_ref)
{
    return tcp_stream_get_runloop(msg_stream_ref->tcp_stream_ref);
}
