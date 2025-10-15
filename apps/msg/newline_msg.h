#ifndef H_newline_msg_H
#define H_newline_msg_H
#include <common/iobuffer.h>
#define NewLineMsgParser_TAG "MSGPSR"
#define NewLineMsg_TAG "MSGTAG"
typedef struct NewLineMsg_s  NewLineMsg, * NewLineMsgRef;
typedef struct NewLineMsgParser_s NewLineMsgParser, *NewLineMsgParserRef;

typedef void(MsgParserCallback)(void* arg, NewLineMsgRef new_msg, int error);

NewLineMsgRef newline_msg_new();
void newline_msg_init(NewLineMsgRef msg_ref);
void newline_msg_deinit(NewLineMsgRef msg_ref);
void newline_msg_free(NewLineMsgRef msg_ref);
IOBufferRef newline_msg_serialize(NewLineMsgRef msg_ref);
IOBufferRef newline_msg_get_content(NewLineMsgRef msg_ref);
void newline_msg_set_content(NewLineMsgRef mr, IOBufferRef iob);

NewLineMsgParserRef newline_msg_parser_new();
void newline_msg_parser_init(NewLineMsgParserRef parser_ref);
void newline_msg_parser_deinit(NewLineMsgParserRef parser_ref);
void newline_msg_parser_free(NewLineMsgParserRef parser_ref);
void newline_msg_parser_consume(NewLineMsgParserRef, IOBufferRef new_data, MsgParserCallback cb/*void(cb)(void* user_ctx, void* new_msg, int error)*/, void* arg);

#define MSG_REF NewLineMsgRef
#define MSG_NEW newline_msg_new()
#define MSG_FREE(msg_ref) newline_msg_free(msg_ref)
#define MSG_SET_CONTENT(msg_ref, content) newline_msg_set_content(msg_ref, content)
#define MSG_GET_CONTENT(msg_ref) (newline_msg_get_content(msg_ref))
#define MSG_SERIALIZE(msg_ref) newline_msg_serialize(msg_ref)

#define MSG_PARSER_REF NewLineMsgParserRef
#define MSG_PARSER_NEW() newline_msg_parser_new()
#define MSG_PARSER_FREE(msg_parser_ref) newline_msg_parser_free(msg_parser_ref)
#define MSG_PARSER_DEINIT(msg_parser_ref) newline_msg_parser_deinit(msg_parser_ref)
#define MSG_PARSER_CONSUME(msg_parser_ref, new_data, msg_callback, arg) newline_msg_parser_consume(msg_parser_ref, new_data, msg_callback, arg)

#endif