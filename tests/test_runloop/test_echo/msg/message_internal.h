#ifndef H_message_internal_H
#define H_message_internal_H
#include <rbl/check_tag.h>
#include <common/iobuffer.h>

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

#endif