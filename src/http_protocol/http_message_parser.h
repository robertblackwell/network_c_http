/**
 * @file message
 * @brief   Module for parsing http/1.1 messages. It does this by providing a wrapper for
 *          github.com/nodejs/llhttp which replaces github.com/joyent/http-parser
 */


#ifndef c_http_http_parser_h
#define c_http_http_parser_h
#include <stdint.h>
#include <llhttp/llhttp.h>
#include <rbl/check_tag.h>
#include <http/parser_types.h>
#include <http_protocol/http_message.h>
#include <common/cbuffer.h>

#define HTTP_PARSER_TAG "HTPARS"

/**
 * state values used in parsing http header lines
 */
#define kHEADER_STATE_NOTHING 10
#define kHEADER_STATE_FIELD   11
#define kHEADER_STATE_VALUE   12

#define Parser_more_data 20
#define Parser_io_error  21
#define Parser_parse_error 22
/**
 * Holds  details of a parse failure
 */
struct http_parser_error_s {
    char*               m_name;
    char*               m_description;
    llhttp_errno_t      m_err_number;
};
typedef struct http_parser_error_s http_parser_error_t;

/**
 * Type holding context data for HttpMessageParser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct HttpMessageParser_s;
typedef struct HttpMessageParser_s HttpMessageParser, *HttpMessageParserRef;
typedef void (*ParserOnMessageCompleteHandler)(void* new_msg_ctx, HttpMessageRef msg);

struct HttpMessageParser_s {
    RBL_DECLARE_TAG;
    bool m_started;

    llhttp_t*                       m_llhttp_ptr;
    llhttp_settings_t*              m_llhttp_settings_ptr;
    HttpMessageRef                  current_message_ptr;
    ParserOnMessageCompleteHandler  on_message_handler;
    void*                           on_message_handler_context;
    int                             m_header_state;
    ///////////////////////////////////////////////////////////////////////////////////
    // String buffers used to accumulate values from http-parser
    ///////////////////////////////////////////////////////////////////////////////////
    CbufferRef                      m_url_buf;
    CbufferRef                      m_status_buf;
    CbufferRef                      m_name_buf;
    CbufferRef                      m_value_buf;
};
void HttpParser_reset(HttpMessageParser*);
HttpMessageParserRef http_message_parser_new(
        // a function that will be called by the parser when parsing of a new message is comlete
        void(*on_new_message_cb)(void* on_new_message_ctx, HttpMessageRef new_message_ref),
        // a pointer to a ctx object you want the handler to have access to while it
        // deciddes what to do with a new message
        void* handler_context) ;
void http_message_parser_free(HttpMessageParserRef this);

/**
 * @brief The guts of the http message parsing process.
 *
 * Call this function repeatedly with successive buffers of data.
 * These successive buffers may represent one or more messages and a single buffer is permitted to hold
 * the end of one message and the start of the successive message.
 *
 * When a full message is complete the parser will call the 'message_complete_handler'. This function should
 * process the message and send the reply before returning. This will suspend the parser until the processing
 * of the message is complete. And if necessary will continue with the next message
 *
 * The http_message_parser_consume() function will only return  if:
 *
 * -    it has consumed all the data. So read some more and give it to consume.
 *
 * -    an error was encountered.
 *      Either a parse error, or an io error - This should be treated as fatal and the connection closed.
 *
 * -    in a situation where a messsage does not contain a message length value the parser may need to be given
 *      an End-Of_File signal in the form of a call to http_message_parser_consume with a zero length buffer.
 *      This need for this will be signalled by llhttp_message_needs_eof(parser->m_llhttp_ptr) returning true
 *
 *
 * @param parser HttpMessageParserRef
 * @param buffer A buffer of data presumable read from a tcp connectin
 * @param length Length of the data buffer
 * @return llhttp_errno_t
 */
llhttp_errno_t          http_message_parser_consume(HttpMessageParser *parser, const void* buffer, int length);
llhttp_errno_t          http_message_parser_consume_buffer(HttpMessageParser *parser, IOBufferRef iobuffer_ref);
llhttp_errno_t          http_message_parser_consume_eof(HttpMessageParser* parser);
llhttp_errno_t          http_message_parser_get_errno(HttpMessageParser* parser);
http_parser_error_t     http_message_parser_get_error(HttpMessageParser *parser);
const void*             http_message_parser_last_byte_parsed(HttpMessageParser* this);

#endif
