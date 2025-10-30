
#ifndef H_stx_msg_h
#define H_stx_msg_h
#include <stdbool.h>
#include <stdint.h>
#include <src/common/buffer_chain.h>

typedef struct StxMsg_s StxMsg, *StxMsgRef;

StxMsgRef stx_msg_new();
void stx_msg_free(StxMsgRef p);
IOBufferRef stx_msg_serialize(StxMsgRef this);
IOBufferRef stx_msg_get_content(StxMsgRef m);
void stx_msg_set_content(StxMsgRef m, IOBufferRef iob);

StxMsgRef stx_msg_new_request();
StxMsgRef stx_msg_new_response();
bool stx_msg_is_request(StxMsgRef mref);
bool stx_msg_get_is_request(StxMsgRef this);
void stx_msg_set_is_request(StxMsgRef this, bool yn);
void stx_msg_set_lrc(StxMsgRef this, char lrc);
void stx_msg_set_content_length(StxMsgRef this, int length);
IOBufferRef stx_msg_get_body(StxMsgRef mref);
void stx_msg_set_body(StxMsgRef mref, IOBufferRef bodyp);

#endif