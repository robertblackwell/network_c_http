#include "msg_generic.h"
inline const char* generic_msg_selection() {
#if defined(MSG_SELECT_NEWLINE)
    return "newline_msg";
#elif defined(MSG_SELECT_STX)
    return "stx_msg";
#elif defined(MSG_SELECT_HTTP)
    return "http_msg";
#endif
}
inline GenericMsgRef generic_msg_new() {
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_new();
#elif defined(MSG_SELECT_STX)
    return stx_msg_new();
#elif defined(MSG_SELECT_HTTP)
    return http_message_new();
#endif
}
inline void generic_msg_free(GenericMsgRef msg_ref){
#if defined(MSG_SELECT_NEWLINE)
    newline_msg_free(msg_ref);
#elif defined(MSG_SELECT_STX)
    stx_msg_free(msg_ref);
#elif defined(MSG_SELECT_HTTP)
    http_message_free(msg_ref);
#endif
}
inline IOBufferRef generic_msg_get_content(GenericMsgRef msg){
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_get_content(msg);
#elif defined(MSG_SELECT_STX)
    return stx_msg_get_content(msg);
#elif defined(MSG_SELECT_HTTP)
    assert(0);
#endif
}
inline void generic_msg_set_content(GenericMsgRef msg, IOBufferRef iob){
#if defined(MSG_SELECT_NEWLINE)
    newline_msg_set_content(msg, iob);
#elif defined(MSG_SELECT_STX)
    stx_msg_set_content(msg, iob);
#elif defined(MSG_SELECT_HTTP)
    assert(0);
#endif
}

inline IOBufferRef generic_msg_serialize(GenericMsgRef mr){
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_serialize(mr);
#elif defined(MSG_SELECT_STX)
    return stx_msg_serialize(mr);
#elif defined(MSG_SELECT_HTTP)
    return http_message_serialize(mr);
#endif
}

inline GenericMsgParserRef generic_msg_parser_new(GenericMsgParserCallback* cb, void* arg){
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_parser_new(cb, arg);
#elif defined(MSG_SELECT_STX)
    return stx_msg_parser_new(cb, arg);
#elif defined(MSG_SELECT_HTTP)
    return http_message_parser_new(cb, arg);
#endif
}
inline void generic_msg_parser_free(GenericMsgParserRef parser_ref){
#if defined(MSG_SELECT_NEWLINE)
    newline_msg_parser_free(parser_ref);
#elif defined(MSG_SELECT_STX)
    stx_msg_parser_free(parser_ref);
#elif defined(MSG_SELECT_HTTP)
    http_message_parser_free(parser_ref);
#endif
}
inline int generic_msg_parser_consume(GenericMsgParserRef pref, IOBufferRef new_data){
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_parser_consume(pref, new_data);
#elif defined(MSG_SELECT_STX)
    return stx_msg_parser_consume(pref, new_data);
#elif defined(MSG_SELECT_HTTP)
    return http_message_parser_consume_buffer(pref, new_data);
#endif
}
inline const char* generic_strerror(GenericMsgParserRef pref, int parser_errno){
#if defined(MSG_SELECT_NEWLINE)
    return newline_msg_parser_strerror(pref, parser_errno);
#elif defined(MSG_SELECT_STX)
    return stx_msg_parser_strerror(pref, parser_errno);
#elif defined(MSG_SELECT_HTTP)
#endif
}
