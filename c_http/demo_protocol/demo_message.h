
#ifndef demo__message_h
#define demo__message_h
#include <stdbool.h>
#include <stdint.h>
#include <c_http/common/buffer_chain.h>

struct DemoMessage_s;
typedef struct DemoMessage_s DemoMessage, *DemoMessageRef, *DemoRequestRef, *DemoResponseRef;

DemoMessageRef demo_message_new();
DemoMessageRef demo_message_new_request();
DemoMessageRef demo_message_new_response();
void demo_message_free(DemoMessageRef p);
void demo_message_dispose(DemoMessageRef* p);
void demo_message_dealloc(void* m);
/**
 * @brief Methods to test and set whether a message is a request or response
 */
bool demo_message_is_request(DemoMessageRef mref);
bool demo_message_get_is_request(DemoMessageRef this);
void demo_message_set_is_request(DemoMessageRef this, bool yn);
void demo_message_set_lrc(DemoMessageRef this, char lrc);

void demo_message_set_content_length(DemoMessageRef this, int length);
BufferChainRef demo_message_get_body(DemoMessageRef mref);
void demo_message_set_body(DemoMessageRef mref, BufferChainRef bodyp);
IOBufferRef demo_message_serialize(DemoMessageRef this);

#endif