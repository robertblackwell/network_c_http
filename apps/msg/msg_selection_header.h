#ifndef H_apps_msg_msg_selection_header_h
#define H_apps_msg_msg_selection_header_h

/**
 * This header is a convenience so client modules do not have to write the conditional
 * to select the correct message module
 */
#if defined(MSG_SELECT_ECHO)
    #warning " on the MSG_SELECT_ECHO branch"
    #include <msg/newline_msg.h>
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

#elif defined(MSG_SELECT_DEMO)
    #warning " on the MSG_SELECT_DEMO branch"

    #include <msg/demo_msg.h>

    #define MSG_REF DemoMsgRef
    #define MSG_NEW demo_msg_new()
    #define MSG_FREE(msg_ref) demo_msg_free(msg_ref)
    #define MSG_SET_CONTENT(msg_ref, content) demo_msg_set_content(msg_ref, content)
    #define MSG_GET_CONTENT(msg_ref) (demo_msg_get_content(msg_ref))
    #define MSG_SERIALIZE(msg_ref) demo_msg_serialize(msg_ref)

    #define MSG_PARSER_REF DemoMsgParserRef
    #define MSG_PARSER_NEW() demo_msg_parser_new()
    #define MSG_PARSER_FREE(msg_parser_ref) demo_msg_parser_free(msg_parser_ref)
    #define MSG_PARSER_DEINIT(msg_parser_ref) demo_msg_parser_deinit(msg_parser_ref)
    #define MSG_PARSER_CONSUME(msg_parser_ref, new_data, msg_callback, arg) demo_msg_parser_consume(msg_parser_ref, new_data, msg_callback, arg)


#else
#error "msg stream - have not selected message type"
#endif

#endif