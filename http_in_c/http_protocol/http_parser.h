#ifndef http_parser_h
#define http_parser_h
#include <stdint.h>
#include <http_in_c/http_protocol/http_message.h>
#include <http_in_c/common/cbuffer.h>


#define HttpParser_TAG "DmPARS"
#include <rbl/check_tag.h>

/**
 * state values used in parsing http header lines
 */
#define kHEADER_STATE_NOTHING 10
#define kHEADER_STATE_FIELD   11
#define kHEADER_STATE_VALUE   12
/**
 * Holds  details of a parse failure
 */
struct HttpParserError_s {
    char*               m_name;
    char*               m_description;
    int                 m_err_number;
};
typedef struct HttpParserError_s HttpParserError;

/**
 * \brief  Return code used as part of the value returned by http_parser_consume() when processing data.
 */
enum HttpParserRC {
    HttpParserRC_end_of_message = 0x01,
    HttpParserRC_message_incomplete = 0x00,
    HttpParserRC_error = -1,
//    HttpParserRC_invalid_opcode,
//    HttpParserRC_expected_stx,
//    HttpParserRC_expected_ascii,
};
typedef int HttpParserErrCode;
#define HttpParserErr_invalid_opcode -11
#define HttpParserErr_expected_stx   -12
#define HttpParserErr_expected_ascii -13

#define HttpParserErr_invalid_opcode_message "invalid opcode"
#define HttpParserErr_expected_stx_message   "expected etx"
#define HttpParserErr_expected_ascii_message "expected printable"

typedef enum HttpParserRC HttpParserRC;

typedef struct ParserInterface_s ParserInterface, *ParserInterfaceRef;

struct ParserInterface_s {
    int(*parser_consume)(ParserInterfaceRef parser_ref, IOBufferRef);
    void*(*message_factory)();
    void(*message_free)(void*);
};
/**
 * Type holding context data for http_parser_t functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct HttpParser_s;
typedef struct HttpParser_s HttpParser, *HttpParserRef;
typedef void(*DP_MessageComplete_CB)(void* ctx, HttpMessageRef);
struct HttpParser_s {
    ParserInterface;
    RBL_DECLARE_TAG;
    int                    m_state;
    void*                  on_new_message_complete_ctx; // generally a pointer to the connection
    DP_MessageComplete_CB  on_new_message_callback;
    HttpMessageRef         m_current_message_ptr;
    RBL_DECLARE_END_TAG;
};

HttpParserRef HttpParser_new(
        /**
         * This function is called every time the parser completes a new message
         */
        DP_MessageComplete_CB on_message_complete_cb,
        /**
         * This is an anonymous pointer to the context object you want the on_message_complete_cb
         * to have while it decides what to do with the new message.
         */
        void* on_new_message_ctx);

void HttpParser_free(HttpParserRef this);

void HttpParser_consume(HttpParserRef parser, IOBufferRef iobuffer_ref);

#endif


/**@}*/