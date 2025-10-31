#ifndef H_generic_msg_h
#define H_generic_msg_h

//#define MSG_SELECT_STX

#if defined(MSG_SELECT_NEWLINE)
#include <src/newline/newline_msg.h>
typedef NewlineMsg GenericMessage;
typedef NewlineMsgRef GenericMsgRef;
typedef NewlineMsgParser GenericMsgParser;
typedef NewlineMsgParserRef GenericMsgParserRef;
typedef NewlineMsgParserCallback GenericMsgParserCallback;

#elif defined(MSG_SELECT_STX)

#include <src/stx/stx_msg.h>
#include <src/stx/stx_msg_parser.h>
typedef StxMsg GenericMessage;
typedef StxMsgRef GenericMsgRef;
typedef StxMsgParser GenericMsgParser;
typedef StxMsgParserRef GenericMsgParserRef;
typedef StxMsgParserCallback GenericMsgParserCallback;

#elif defined(MSG_SELECT_HTTP)

#include <src/http/http_message.h>
#include <src/http/http_message_parser.h>
#include <apps/http_common/http_make_request_response.h>
typedef HttpMessage GenericMessage;
typedef HttpMessageRef GenericMsgRef;
typedef HttpMessageParser GenericMsgParser;
typedef HttpMessageParserRef GenericMsgParserRef;
//typedef HttpMessageParserCallback GenericMsgParserCallback;
typedef ParserOnMessageCompleteHandler GenericMsgParserCallback;
#endif
const char* generic_msg_selection();
GenericMsgRef generic_msg_new();
void generic_msg_free(GenericMsgRef msg_ref);
IOBufferRef generic_msg_get_content(GenericMsgRef msg);
void generic_msg_set_content(GenericMsgRef msg, IOBufferRef iob);
IOBufferRef generic_msg_serialize(GenericMsgRef mr);

GenericMsgParserRef generic_msg_parser_new(GenericMsgParserCallback* cb, void* arg);
void generic_msg_parser_free(GenericMsgParserRef parser_ref);
int generic_msg_parser_consume(GenericMsgParserRef pref, IOBufferRef new_data);
const char* generic_strerror(GenericMsgParserRef parser_ref, int parser_errno);

#endif