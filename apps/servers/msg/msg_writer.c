#include <msg/msg_stream.h>
static void tcp_write_callback(void* arg, int error);
static void invoke_write_callback(MsgStreamRef msg_stream_ref, int error);

void msg_stream_write(MsgStreamRef msg_stream_ref, MSG_REF msg, MsgWriteCallback write_cb, void* arg)
{
    assert(msg_stream_ref->write_cb == NULL);
    assert(msg_stream_ref->write_cb_arg == NULL);
    assert(msg_stream_ref->output_buffer == NULL);
    msg_stream_ref->write_cb = write_cb;
    msg_stream_ref->write_cb_arg = arg;
    msg_stream_ref->output_buffer = MSG_SERIALIZE(msg);
    tcp_write(msg_stream_ref->tcp_stream_ref, msg_stream_ref->output_buffer, tcp_write_callback, msg_stream_ref);
}
static void tcp_write_callback(void* arg, int error)
{
    MsgStreamRef msg_stream_ref = arg;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref)
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref)
    msg_stream_ref->output_buffer = NULL;
    invoke_write_callback(msg_stream_ref, error);
}
static void invoke_write_callback(MsgStreamRef msg_stream_ref, int error)
{
    MsgWriteCallback* wcb = msg_stream_ref->write_cb;
    void* arg = msg_stream_ref->write_cb_arg;
    msg_stream_ref->write_cb = NULL;
    msg_stream_ref->write_cb_arg = NULL;
    wcb(arg, error);
}   