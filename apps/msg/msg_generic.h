#ifndef H_generic_msg_h
#define H_generic_msg_h

#if defined(MSG_SELECT_NEWLINE)
#include "newline/newline_msg.h"
typedef NewlineMsg GenericMessage;
typedef NewlineMsgRef GenericMsgRef;
typedef NewlineMsgParser GenericMsgParser;
typedef NewlineMsgParserRef GenericMsgParserRef;
typedef NewlineMsgParserCallback GenericMsgParserCallback;
#elif defined(MSG_SELECT_STX)
#include "apps/msg/stx/stx_msg.h"
#include "apps/msg/stx/stx_msg_parser.h"
typedef StxMsg GenericMessage;
typedef StxMsgRef GenericMsgRef;
typedef StxMsgParser GenericMsgParser;
typedef StxMsgParserRef GenericMsgParserRef;
typedef StxMsgParserCallback GenericMsgParserCallback;
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