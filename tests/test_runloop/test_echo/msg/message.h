#ifndef H_message_H
#define H_message_H
#include <common/iobuffer.h>
#define MsgParser_TAG "MSGPSR"
#define Message_TAG "MSGTAG"
typedef struct Message_s  Message, * MessageRef;
typedef struct MsgParser_s MsgParser, *MsgParserRef;

typedef void(MsgReadCallback)(void* arg, MessageRef new_msg, int error);
typedef void(MsgWriteCallback)(void* arg, int error);
typedef void(MsgParserCallback)(void* arg, MessageRef new_msg, int error);

MessageRef message_new();
void message_init(MessageRef msg_ref);
void message_deinit(MessageRef msg_ref);
void message_free(MessageRef msg_ref);
IOBufferRef message_serialize(MessageRef msg_ref);
IOBufferRef message_get_content(MessageRef msg_ref);
void message_set_content(MessageRef mr, IOBufferRef iob);

MsgParserRef msg_parser_new();
void msg_parser_init(MsgParserRef parser_ref);
void msg_parser_deinit(MsgParserRef parser_ref);
void msg_parser_free(MsgParserRef parser_ref);
void msg_parser_consume(MsgParserRef, IOBufferRef new_data, MsgParserCallback cb, void* arg);

#endif