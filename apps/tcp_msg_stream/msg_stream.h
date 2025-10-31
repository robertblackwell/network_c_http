#ifndef H_msg_stream_h
#define H_msg_stream_h
#include <runloop/runloop.h>
#include <common/list.h>
#include <src/tcp/tcp_stream.h>
#include <apps/msg/msg_generic.h>
#define MsgStream_TAG "MSGSTR"

typedef struct MsgStream_s MsgStream, *MsgStreamRef;
typedef void(MsgReadCallback)(void* arg, GenericMsgRef msg, int error);
typedef void(MsgWriteCallback)(void* arg, int error);

struct MsgStream_s {
    RBL_DECLARE_TAG;
    TcpStreamRef    tcp_stream_ref;
    GenericMsgParserRef    msg_parser_ref;

    void*             read_cb_arg;
    MsgReadCallback*  read_cb;
    IOBufferRef       input_buffer;
    ListRef           input_message_list;
    IOBufferRef       output_buffer;
    MsgWriteCallback* write_cb;
    void*             write_cb_arg;
    RBL_DECLARE_END_TAG;
};

MsgStreamRef msg_stream_new(RunloopRef rl, int fd);
void msg_stream_init(MsgStreamRef msg_stream_ref, RunloopRef rl, int fd);
void msg_stream_deinit(MsgStreamRef msg_stream_ref);
void msg_stream_free(MsgStreamRef msg_stream_ref);
/*
 * This function passes a MessageRef to the caller via the callback.
 * That is a transfer of ownership and the caller is resposible for
 * deallocating the message object if necessary, error == 0 means success.
 * When error != 0 for an io error msg will be NULL.
 * An error while parsing a message will result in
 * error != 0 and a partially complete message from which it may be possible
 * to determine where the parse error occurred. This will depend on the actual
 * real message type
 */
void msg_stream_read(MsgStreamRef msg_stream_ref, MsgReadCallback read_cb, void* arg);
/*
 * This function may modify the msg_ref while framing for transmission but ownership
 * remains with the caller. After write_cb is invoked the caller should manage the msg_ref
 * lifetime as it needs.
 */
void msg_stream_write(MsgStreamRef msg_stream_ref, GenericMsgRef msg_ref, MsgWriteCallback write_cb, void* arg);

#endif