#include "msg_stream.h"
#include <rbl/macros.h>
static void postable_read(RunloopRef rl, void* arg);
static void tcp_read_callback(void* arg, int error);
static void new_message_callback(void* msgstream, GenericMsgRef new_msg, int error);
static void invoke_read_callback(MsgStreamRef msg_stream_ref, GenericMsgRef new_msg, int error);
static void try_read(MsgStreamRef msg_stream_ref);

void msg_stream_reader_init(MsgStreamRef ms)
{
#if 0
    ms->msg_parser_ref = generic_msg_parser_new(new_message_callback, ms);
#else
    ms->msg_parser_ref = &(ms->msg_parser_mem);
    generic_msg_parser_init(ms->msg_parser_ref, new_message_callback, ms);
#endif
}
void msg_stream_read(MsgStreamRef msg_stream, MsgReadCallback cb, void* arg)
{
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream);
    assert(msg_stream->read_cb == NULL);
    // printf("msg_stream_read input_buffer: %p\n", msg_stream->input_buffer);
    // if(msg_stream->input_buffer == NULL) {
    //     msg_stream->input_buffer = IOBuffer_new();
    // }
    msg_stream->read_cb = cb;
    msg_stream->read_cb_arg = arg;
    if (List_size(msg_stream->input_message_list) != 0) {
        GenericMsgRef m = List_remove_first(msg_stream->input_message_list);
        invoke_read_callback(msg_stream, m, 0);
    } else {
        // printf("msg_stream_read input_buffer: %p\n", msg_stream->input_buffer);
        if(msg_stream->input_buffer == NULL) {
            msg_stream->input_buffer = IOBuffer_new();
        }
        TcpStreamRef tcp_stream = msg_stream->tcp_stream_ref;
        tcp_read(tcp_stream, msg_stream->input_buffer, tcp_read_callback, msg_stream);
    }
}
/*
 * Enter this with new data in msg_stream_ref->input_buffer
 */
static void tcp_read_callback(void* arg, int error)
{
    MsgStreamRef msg_stream_ref = arg;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    RunloopRef rl = runloop_stream_get_runloop(msg_stream_ref->tcp_stream_ref->rlstream_ref);
    if(error == 0) {
        RBL_ASSERT((IOBuffer_data_len(msg_stream_ref->input_buffer) != 0), "inpput_buffer null - failed invariant");
        // printf("tcp_read_callback - got buffer %s\n", IOBuffer_cstr(msg_stream_ref->input_buffer));
        int parser_error = generic_msg_parser_consume(msg_stream_ref->msg_parser_ref, msg_stream_ref->input_buffer);//, new_message_callback, msg_stream_ref);
        if(parser_error == 0) {
            assert(IOBuffer_data_len(msg_stream_ref->input_buffer) == 0);
#if 1
            IOBuffer_free(msg_stream_ref->input_buffer);
            msg_stream_ref->input_buffer = NULL;
#else
            IOBuffer_reset(msg_stream_ref->input_buffer);
#endif
            if (List_size(msg_stream_ref->input_message_list) != 0) {
                GenericMsgRef m = List_remove_first(msg_stream_ref->input_message_list);
                invoke_read_callback(msg_stream_ref, m, error);
            } else {
                // printf("tcp_read_callback input list was empty read again input_buffer: %p\n", msg_stream_ref->input_buffer);
                msg_stream_ref->input_buffer = IOBuffer_new();
                runloop_post(rl, postable_read, msg_stream_ref);
            }
        } else {
            assert(0);
            IOBuffer_reset(msg_stream_ref->input_buffer);
            invoke_read_callback(msg_stream_ref, NULL, parser_error);
        }
    } else {
        invoke_read_callback(msg_stream_ref, NULL, error);
    }
}
static void new_message_callback(void* msgstream, GenericMsgRef new_msg, int error)
{
    MsgStreamRef msg_stream_ref = msgstream;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    // printf("new_message_callback new_msg: %p\n", new_msg);
    if(error == 0) {
        List_add_back(msg_stream_ref->input_message_list, new_msg);
    } else {
        printf("new_message_callback error:%d new_msg: %p\n", error, new_msg);
    }
}
static void postable_read(RunloopRef rl, void* arg)
{
    MsgStreamRef msg_stream_ref = arg;
    RBL_CHECK_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_CHECK_END_TAG(MsgStream_TAG, msg_stream_ref);
    RBL_ASSERT((msg_stream_ref->input_buffer != NULL), "non-null input buffer ");
    try_read(msg_stream_ref);
}
static void try_read(MsgStreamRef msg_stream_ref)
{
    TcpStreamRef tcp_stream = msg_stream_ref->tcp_stream_ref;
    tcp_read(tcp_stream, msg_stream_ref->input_buffer, tcp_read_callback, msg_stream_ref); 
}
static void invoke_read_callback(MsgStreamRef msg_stream_ref, GenericMsgRef new_msg, int error)
{
    MsgReadCallback* cb = msg_stream_ref->read_cb;
    void* arg = msg_stream_ref->read_cb_arg;
    msg_stream_ref->read_cb = NULL;
    msg_stream_ref->read_cb_arg = NULL;
    cb(arg, new_msg, error);
}