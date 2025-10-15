#ifndef H_msg_select_H
#define H_msg_select_H

#if defined(MSG_SELECT_ECHO)
    #include "newline_msg.h"
#elif defined(MSG_SELECT_DEMO)
    #include "demo_message.h"
#else
    #error "message select failed
#endif


#endif