#ifndef demo_parser_h
#define demo_parser_h
#include <stdint.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/common/cbuffer.h>


#define DemoParser_TAG "DmPARS"
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
struct DemoParserError_s {
    char*               m_name;
    char*               m_description;
    int                 m_err_number;
};
typedef struct DemoParserError_s DemoParserError;

/**
 * \brief  Return code used as part of the value returned by HttpParser_consume() when processing data.
 */
enum DemoParserRC {
    DemoParserRC_end_of_message = 0x01,
    DemoParserRC_message_incomplete = 0x00,
    DemoParserRC_error = -1,
//    DemoParserRC_invalid_opcode,
//    DemoParserRC_expected_stx,
//    DemoParserRC_expected_ascii,
};
typedef int DemoParserErrCode;
#define DemoParserErr_invalid_opcode -11
#define DemoParserErr_expected_stx   -12
#define DemoParserErr_expected_ascii -13

#define DemoParserErr_invalid_opcode_message "invalid opcode"
#define DemoParserErr_expected_stx_message   "expected etx"
#define DemoParserErr_expected_ascii_message "expected printable"

typedef enum DemoParserRC DemoParserRC;

typedef struct ParserInterface_s ParserInterface, *ParserInterfaceRef;

struct ParserInterface_s {
    int(*parser_consume)(ParserInterfaceRef parser_ref, IOBufferRef);
    void*(*message_factory)();
    void(*message_free)(void*);
};
/**
 * Type holding context data for HttpParser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct DemoParser_s;
typedef struct DemoParser_s DemoParser, *DemoParserRef;
typedef void(*DP_MessageComplete_CB)(void* ctx, DemoMessageRef);
struct DemoParser_s {
    ParserInterface;
    RBL_DECLARE_TAG;
    int                    m_state;
    void*                  on_new_message_complete_ctx; // generally a pointer to the connection
    DP_MessageComplete_CB  on_new_message_callback;
    DemoMessageRef         m_current_message_ptr;
    RBL_DECLARE_END_TAG;
};

DemoParserRef DemoParser_new(
        /**
         * This function is called every time the parser completes a new message
         */
        DP_MessageComplete_CB on_message_complete_cb,
        /**
         * This is an anonymous pointer to the context object you want the on_message_complete_cb
         * to have while it decides what to do with the new message.
         */
        void* on_new_message_ctx);

void DemoParser_free(DemoParserRef this);

void DemoParser_consume(DemoParserRef parser, IOBufferRef iobuffer_ref);

#endif


/**@}*/