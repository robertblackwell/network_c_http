#include <kqueue_runloop/runloop.h>
#include <tcp/tcp_stream.h>
#include "msg_stream.h"
static void postable_read(RunloopRef rl, void* arg);
static void tcp_read_callback(void* arg, int error);
static void new_message_callback(void* msgstream, MessageRef new_msg, int error);
static void invoke_read_callback(MsgStreamRef msg_stream_ref, MessageRef new_msg, int error);
static void try_read(MsgStreamRef msg_stream_ref);


void msg_stream_read(MsgStreamRef msg_stream, MsgReadCallback cb, void* arg)
{
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream);
    assert(msg_stream->read_cb == NULL);
    if(msg_stream->input_buffer == NULL) {
        msg_stream->input_buffer = IOBuffer_new();
    }
    msg_stream->read_cb = cb;
    msg_stream->read_cb_arg = arg;
    if (List_size(msg_stream->input_message_list) != 0) {
        MessageRef m = List_remove_first(msg_stream->input_message_list);
        invoke_read_callback(msg_stream, m, 0);
    } else {
        TcpStreamRef tcp_stream = msg_stream->tcp_stream_ref;
        tcp_read(tcp_stream, msg_stream->input_buffer, tcp_read_callback, msg_stream);
    }
}
static void tcp_read_callback(void* arg, int error)
{
    MsgStreamRef msg_stream_ref = arg;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    RunloopRef rl = msg_stream_get_runloop(msg_stream_ref);
    if(error == 0) {
        assert(IOBuffer_data_len(msg_stream_ref->input_buffer) != 0);
        msg_parser_consume(msg_stream_ref->msg_parser_ref, msg_stream_ref->input_buffer, new_message_callback, msg_stream_ref);
        assert(IOBuffer_data_len(msg_stream_ref->input_buffer) == 0);
        if (List_size(msg_stream_ref->input_message_list) != 0) {
            MessageRef m = List_remove_first(msg_stream_ref->input_message_list);
            invoke_read_callback(msg_stream_ref, m, error);
        } else {
            runloop_post(rl, postable_read, msg_stream_ref);
        }
    } else {
        invoke_read_callback(msg_stream_ref, NULL, error);
    }
}
static void new_message_callback(void* msgstream, MessageRef new_msg, int error)
{
    MsgStreamRef msg_stream_ref = msgstream;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    if(error == 0) {
        List_add_back(msg_stream_ref->input_message_list, new_msg);
    }
}
static void postable_read(RunloopRef rl, void* arg)
{
    MsgStreamRef msg_stream_ref = arg;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    try_read(msg_stream_ref);
}
static void try_read(MsgStreamRef msg_stream_ref)
{
    TcpStreamRef tcp_stream = msg_stream_ref->tcp_stream_ref;
    tcp_read(tcp_stream, msg_stream_ref->input_buffer, tcp_read_callback, msg_stream_ref); 
}
static void invoke_read_callback(MsgStreamRef msg_stream_ref, MessageRef new_msg, int error)
{
    MsgReadCallback* cb = msg_stream_ref->read_cb;
    void* arg = msg_stream_ref->read_cb_arg;
    msg_stream_ref->read_cb = NULL;
    msg_stream_ref->read_cb_arg = NULL;
    cb(arg, new_msg, error);
}