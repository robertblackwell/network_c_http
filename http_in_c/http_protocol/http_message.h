
#ifndef http__message_h
#define http__message_h
#include <stdbool.h>
#include <stdint.h>
#include <http_in_c/common/buffer_chain.h>

struct HttpMessage_s;
typedef struct HttpMessage_s HttpMessage, *HttpMessageRef, *HttpRequestRef, *HttpResponseRef;

HttpMessageRef http_message_new();
HttpMessageRef http_message_new_request();
HttpMessageRef http_message_new_response();
void http_message_free(HttpMessageRef p);
/**
 * @brief Methods to test and set whether a message is a request or response
 */
bool http_message_is_request(HttpMessageRef mref);
bool http_message_get_is_request(HttpMessageRef this);
void http_message_set_is_request(HttpMessageRef this, bool yn);
void http_message_set_lrc(HttpMessageRef this, char lrc);
void http_message_set_content_length(HttpMessageRef this, int length);
BufferChainRef http_message_get_body(HttpMessageRef mref);
void http_message_set_body(HttpMessageRef mref, BufferChainRef bodyp);
IOBufferRef http_message_serialize(HttpMessageRef this);

#endif