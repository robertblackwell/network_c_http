#ifndef H_stx_msg_parser_h
#define H_stx_msg_parser_h
#include <stdint.h>
#include "stx_msg.h"
#include <src/common/cbuffer.h>


#define StxMsgParser_TAG "StxPAR"
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
struct StxMsgParserError_s {
    char*               m_name;
    char*               m_description;
    int                 m_err_number;
};
typedef struct StxMsgParserError_s StxMsgParserError;

/**
 * \brief  Return code used as part of the value returned by http_message_parser_consume() when processing data.
 */
enum StxMsgParserRC {
    StxMsgParserRC_end_of_message = 0x01,
    StxMsgParserRC_message_incomplete = 0x00,
    StxMsgParserRC_error = -1,
//    StxMsgParserRC_invalid_opcode,
//    StxMsgParserRC_expected_stx,
//    StxMsgParserRC_expected_ascii,
};
typedef int StxMsgParserErrCode;
#define StxMsgParserErr_invalid_opcode -11
#define StxMsgParserErr_expected_stx   -12
#define StxMsgParserErr_expected_ascii -13

#define StxMsgParserErr_invalid_opcode_message "invalid opcode"
#define StxMsgParserErr_expected_stx_msg   "expected etx"
#define StxMsgParserErr_expected_ascii_message "expected printable"

typedef enum DemoMessageParserRC DemoMessageParserRC;

typedef struct ParserInterface_s ParserInterface, *ParserInterfaceRef;

/**
 * Type holding context data for HttpMessageParser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
typedef struct StxMsgParser_s StxMsgParser, *StxMsgParserRef;
typedef void(StxMsgParserCallback)(void* arg, StxMsgRef mref, int error);
struct StxMsgParser_s {
    RBL_DECLARE_TAG;
    int                    m_state;
    void*                  on_new_message_complete_ctx; // generally a pointer to the connection
    StxMsgParserCallback   *on_new_message_callback;
    StxMsgRef              m_current_message_ptr;
    RBL_DECLARE_END_TAG;
};

StxMsgParserRef stx_msg_parser_new(
        /**
         * This function is called every time the parser completes a new message
         */
        StxMsgParserCallback  on_message_complete_cb,
        /**
         * This is an anonymous pointer to the context object you want the on_message_complete_cb
         * to have while it decides what to do with the new message.
         */
        void* on_new_message_ctx);

void stx_msg_parser_free(StxMsgParserRef this);

int stx_msg_parser_consume(StxMsgParserRef parser, IOBufferRef iobuffer_ref);

#endif


/**@}*/