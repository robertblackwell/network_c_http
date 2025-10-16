#ifndef H_msg_stream_h
#define H_msg_stream_h
#include <kqueue_runloop/runloop.h>
#include <tcp/tcp_stream.h>
#include <msg/msg_selection_header.h>

#define MsgStream_TAG "MSGSTR"

typedef void(MsgReadCallback)(void* arg, MSG_REF msg, int error);
typedef void(MsgWriteCallback)(void* arg, int error);

typedef struct MsgStream_s {
    RBL_DECLARE_TAG;
    TcpStreamRef      tcp_stream_ref;

    MSG_PARSER_REF    msg_parser_ref;

    void*             read_cb_arg;
    MsgReadCallback*  read_cb;
    IOBufferRef       input_buffer;
    // input_msg should only be none null if the parser has set this to a new message
    // via the parser callback
    ListRef           input_message_list;
    IOBufferRef       output_buffer;
    MsgWriteCallback* write_cb;
    void*             write_cb_arg;
    RBL_DECLARE_END_TAG; 
} MsgStream, *MsgStreamRef;


MsgStreamRef msg_stream_new(RunloopRef rl, int fd);
void msg_stream_init(MsgStreamRef msg_stream_ref, RunloopRef rl, int fd);
// void msg_stream_deinit(MsgStreamRef msg_stream_ref);
void msg_stream_free(MsgStreamRef msg_stream_ref);
RunloopRef msg_stream_get_runloop(MsgStreamRef msg_stream_ref);
/*
 * This function passes a message ref to the caller via the callback.
 * That is a transfer of ownership and the caller is resposible for
 * deallocating if necessary
 */
void msg_stream_read(MsgStreamRef msg_stream_ref, MsgReadCallback read_cb, void* arg);
/*
 * This function may modify the msg_ref while framing for transmission but ownership
 * remains with the caller
 */
void msg_stream_write(MsgStreamRef msg_stream_ref, MSG_REF msg_ref, MsgWriteCallback write_cb, void* arg);


#endif