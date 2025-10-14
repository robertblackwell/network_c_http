#ifndef H_interface_message_h
#define H_interface_message_h
#include <common/iobuffer.h>

typedef struct IMessage
{
    void* (*message_new)();
    void (*message_free)(void*);
    IOBufferRef (*serialize)(void* msg_ref);
    IOBufferRef (*get_content)(void* msg_ref);
    void (*set_content)(void* msg_ref, IOBufferRef iob);
    void* (*parser_new)();
    void (*parser_free)(void* parser_ref);
    void (*parser_consume)(void* parser_ref, IOBufferRef new_data, void(void* user_ctx, void* new_msg_ref, int error), void* arg);
} IMessage, *IMessageRef;


#endif