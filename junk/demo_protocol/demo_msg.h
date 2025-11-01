
#ifndef H_demo_message_h
#define H_demo_message_h
#include <stdbool.h>
#include <stdint.h>
#include <src/common/buffer_chain.h>

struct Message_s;
typedef struct Message_s Message, *MessageRef, *RequestRef, *ResponseRef;

MessageRef demo_message_new();
MessageRef demo_message_new_request();
MessageRef demo_message_new_response();
void message_free(MessageRef p);
void message_anonymous_free(void* p);
bool message_is_request(MessageRef mref);
bool message_get_is_request(MessageRef this);
void message_set_is_request(MessageRef this, bool yn);
void message_set_lrc(MessageRef this, char lrc);
void message_set_content_length(MessageRef this, int length);

BufferChainRef message_get_body(MessageRef mref);
void message_set_body(MessageRef mref, BufferChainRef bodyp);
IOBufferRef demo_message_serialize(MessageRef this);

#endif