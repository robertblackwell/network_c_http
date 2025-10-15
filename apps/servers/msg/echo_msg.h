#ifndef H_echo_msg_H
#define H_echo_msg_H
#include <common/iobuffer.h>
#define EchoMsgParser_TAG "MSGPSR"
#define EchoMsg_TAG "MSGTAG"
typedef struct EchoMsg_s  EchoMsg, * EchoMsgRef;
typedef struct EchoMsgParser_s EchoMsgParser, *EchoMsgParserRef;

typedef void(MsgParserCallback)(void* arg, EchoMsgRef new_msg, int error);

EchoMsgRef echo_msg_new();
void echo_msg_init(EchoMsgRef msg_ref);
void echo_msg_deinit(EchoMsgRef msg_ref);
void echo_msg_free(EchoMsgRef msg_ref);
IOBufferRef echo_msg_serialize(EchoMsgRef msg_ref);
IOBufferRef echo_msg_get_content(EchoMsgRef msg_ref);
void echo_msg_set_content(EchoMsgRef mr, IOBufferRef iob);

EchoMsgParserRef echo_msg_parser_new();
void echo_msg_parser_init(EchoMsgParserRef parser_ref);
void echo_msg_parser_deinit(EchoMsgParserRef parser_ref);
void echo_msg_parser_free(EchoMsgParserRef parser_ref);
void echo_msg_parser_consume(EchoMsgParserRef, IOBufferRef new_data, MsgParserCallback cb/*void(cb)(void* user_ctx, void* new_msg, int error)*/, void* arg);

#define MSG_REF EchoMsgRef
#define MSG_NEW echo_msg_new()
#define MSG_FREE(msg_ref) echo_msg_free(msg_ref)
#define MSG_SET_CONTENT(msg_ref, content) echo_msg_set_content(msg_ref, content)
#define MSG_GET_CONTENT(msg_ref) (echo_msg_get_content(msg_ref))
#define MSG_SERIALIZE(msg_ref) echo_msg_serialize(msg_ref)

#define MSG_PARSER_REF EchoMsgParserRef
#define MSG_PARSER_NEW() echo_msg_parser_new()
#define MSG_PARSER_FREE(msg_parser_ref) echo_msg_parser_free(msg_parser_ref)
#define MSG_PARSER_DEINIT(msg_parser_ref) echo_msg_parser_deinit(msg_parser_ref)
#define MSG_PARSER_CONSUME(msg_parser_ref, new_data, msg_callback, arg) echo_msg_parser_consume(msg_parser_ref, new_data, msg_callback, arg)

#endif