
#ifndef H_demo_message_h
#define H_demo_message_h
#include <stdbool.h>
#include <stdint.h>
#include <src/common/buffer_chain.h>

struct  DemoMsg_s;
typedef struct  DemoMsg_s  DemoMsg, * DemoMsgRef, *DemoRequestRef, *DemoResponseRef;
typedef struct DemoMsgParser_s DemonMsgParser, *DemoMsgParserRef;

 DemoMsgRef demo_msg_new();
 DemoMsgRef demo_msg_new_request();
 DemoMsgRef demo_msg_new_response();
void demo_msg_free( DemoMsgRef p);
void demo_msg_anonymous_free(void* p);
bool demo_msg_is_request( DemoMsgRef mref);
bool demo_msg_get_is_request( DemoMsgRef this);
void demo_msg_set_is_request( DemoMsgRef this, bool yn);
void demo_msg_set_lrc( DemoMsgRef this, char lrc);
void demo_msg_set_content_length( DemoMsgRef this, int length);

IOBufferRef demo_msg_get_content( DemoMsgRef mref);
void demo_msg_set_content(DemoMsgRef msg, IOBufferRef content);

BufferChainRef demo_msg_get_body( DemoMsgRef mref);
void demo_msg_set_body( DemoMsgRef mref, BufferChainRef bodyp);
IOBufferRef demo_msg_serialize( DemoMsgRef this);

typedef struct DemoMsgParser_s DemoMsgParser, *DemoMsgParserRef;
typedef void(*DemoMsgCallback)(void* ctx, DemoMsgRef, int error);

DemoMsgParserRef demo_msg_parser_new();
void demo_msg_parser_free(DemoMsgParserRef this);
void demo_msg_parser_consume(DemoMsgParserRef parser, IOBufferRef iobuffer_ref,
    void (*new_msg_cb)(void *, DemoMsgRef, int error),
    void* on_new_message_ctx
    );


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


#endif