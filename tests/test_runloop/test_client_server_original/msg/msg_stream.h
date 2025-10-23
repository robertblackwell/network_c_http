#ifndef H_msg_stream_h
#define H_msg_stream_h
#include <runloop/runloop.h>
#include <common/list.h>
#include <tcp/tcp_stream.h>

#define MsgStream_TAG "MSGSTR"
#define MsgParser_TAG "MSGPSR"
#define Message_TAG "MSGTAG"


typedef struct Message_s {
    RBL_DECLARE_TAG;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
} Message, * MessageRef;      

typedef void(MsgReadCallback)(void* arg, MessageRef msg, int error);
typedef void(MsgWriteCallback)(void* arg, int error);

typedef struct MsgParser_s {
    RBL_DECLARE_TAG;
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
    MessageRef msg_ref;
    IOBufferRef iob;
    RBL_DECLARE_END_TAG;
} MsgParser, *MsgParserRef;

typedef void(MsgParserCallback)(void* arg, MessageRef msg, int error);

typedef struct MsgReader_s {
    TcpStreamRef   tcp_stream_ref;
    MsgParser      msg_parser_ref;
    IOBufferRef    input_buffer;
    MessageRef input_msg;
    IOBufferRef    output_buffer;
} MsgReader, *MsgWriterRef;

typedef struct MsgWriter_s {

} MsgWriter, *MsgReaderRef;

typedef struct MsgStream_s {
    RBL_DECLARE_TAG;
    TcpStreamRef tcp_stream_ref;
    MsgParserRef    msg_parser_ref;

    void*             read_cb_arg;
    MsgReadCallback*  read_cb;
    IOBufferRef       input_buffer;
    // input_msg should only be none null if the parser has set this to a new message
    // via the parser callback
#if 0
    MessageRef        input_msg;
#else
    ListRef           input_message_list;
#endif
    IOBufferRef       output_buffer;
    MsgWriteCallback* write_cb;
    void*             write_cb_arg;
    RBL_DECLARE_END_TAG; 
} MsgStream, *MsgStreamRef;


MsgStreamRef msg_stream_new(RunloopRef rl, int fd);
void msg_stream_init(MsgStreamRef msg_stream_ref, RunloopRef rl, int fd);
void msg_stream_deinit(MsgStreamRef msg_stream_ref);
void msg_stream_free(MsgStreamRef msg_stream_ref);
/*
 * This function passes a MessageRef to the caller via the callback.
 * That is a transfer of ownership and the caller is resposible for
 * deallocating if necessary
 */
void msg_stream_read(MsgStreamRef msg_stream_ref, MsgReadCallback read_cb, void* arg);
/*
 * This function may modify the msg_ref while framing for transmission but ownership
 * remains with the caller
 */
void msg_stream_write(MsgStreamRef msg_stream_ref, MessageRef msg_ref, MsgWriteCallback write_cb, void* arg);

MessageRef message_new();
void message_init(MessageRef msg_ref);
void message_deinit(MessageRef msg_ref);
void message_free(MessageRef msg_ref);

/**
 * Does not modify mr - ownership stays with caller
 */
IOBufferRef message_serialize(MessageRef mr);

MsgParserRef msg_parser_new();
void msg_parser_init(MsgParserRef parser_ref);
void msg_parser_deinit(MsgParserRef parser_ref);
void msg_parser_free(MsgParserRef parser_ref);
void msg_parser_consume(MsgParserRef, IOBufferRef new_data, MsgParserCallback cb, void* arg);

#endif