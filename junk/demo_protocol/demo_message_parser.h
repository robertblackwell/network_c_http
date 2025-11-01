#ifndef demo_parser_h
#define demo_parser_h
#include <stdint.h>
#include "demo_msg.h"
#include <src/common/cbuffer.h>


#define Parser_TAG "DmPARS"
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
struct ParserError_s {
    char*               m_name;
    char*               m_description;
    int                 m_err_number;
};
typedef struct ParserError_s ParserError;

/**
 * \brief  Return code used as part of the value returned by http_message_parser_consume() when processing data.
 */
enum MessageParserRC {
    ParserRC_end_of_message = 0x01,
    ParserRC_message_incomplete = 0x00,
    ParserRC_error = -1,
//    ParserRC_invalid_opcode,
//    ParserRC_expected_stx,
//    ParserRC_expected_ascii,
};
typedef int ParserErrCode;
#define ParserErr_invalid_opcode -11
#define ParserErr_expected_stx   -12
#define ParserErr_expected_ascii -13

#define ParserErr_invalid_opcode_message "invalid opcode"
#define ParserErr_expected_stx_message   "expected etx"
#define ParserErr_expected_ascii_message "expected printable"

typedef enum MessageParserRC MessageParserRC;

typedef struct ParserInterface_s ParserInterface, *ParserInterfaceRef;

struct ParserInterface_s {
    int  (*parser_consume)(ParserInterfaceRef parser_ref, IOBufferRef);
    void*(*message_factory)();
    void (*message_free)(void*);
};
/**
 * Type holding context data for HttpMessageParser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct MessageParser_s;
typedef struct MessageParser_s MessageParser, *MessageParserRef;
typedef void(*DP_MessageComplete_CB)(void* ctx, MessageRef);
struct MessageParser_s {
    ParserInterface;
    RBL_DECLARE_TAG;
    int                     m_state;
    void*                   on_new_message_complete_ctx; // generally a pointer to the connection
    DP_MessageComplete_CB   on_new_message_callback;
    MessageRef              m_current_message_ptr;
    RBL_DECLARE_END_TAG;
};

MessageParserRef message_parser_new();
    // /**
    //  * This function is called every time the parser completes a new message
    //  */
    // void(*on_message_complete_cb)(void* on_msg_ctx, MessageRef msgref),
    // /**
    //  * This is an anonymous pointer to the context object you want the on_message_complete_cb
    //  * to have while it decides what to do with the new message.
    //  */
    // void* on_new_message_ctx);

void message_parser_free(MessageParserRef this);

int message_parser_consume(MessageParserRef parser,
    IOBufferRef iobuffer_ref,
    void (*on_message_complete_cb)(void *, MessageRef),
    void* on_new_message_ctx
    );

#endif


/**@}*/