#ifndef H_apps_msg_msg_selection_header_h
#define H_apps_msg_msg_selection_header_h

/**
 * This header is a convenience so client modules do not have to write the conditional
 * to select the correct message module
 */
#if defined(MSG_SELECT_ECHO)
    #include <msg/newline_msg.h>
#elif defined(MSG_SELECT_DEMO)
    #include <msg/demo_msg.h>
#else
#error "msg stream - have not selected message type"
#endif

#endif