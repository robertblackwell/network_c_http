#ifndef demo_parser_h
#define demo_parser_h
#include <stdint.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/common/cbuffer.h>


#define DemoParser_TAG "DmPARS"
#include <http_in_c/check_tag.h>

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
 * \brief  Return code used as part of the value returned by http_parser_consume() when processing data.
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

#if 0
/**
 * Value object return by http_parser_consume()
 */
struct DemoParserPrivateReturnValue_s {
//    long            bytes_consumed;
//    bool            eom_flag;
    int             error_code;
};
typedef struct DemoParserPrivateReturnValue_s DemoParserPrivateReturnValue;
//typedef struct DemoParserReturnValue_s {
//    void*       completed_message_ref;
//    IOBufferRef remaining_data_ref;
//    bool        error_flag;
//    int         error_code;
//}DemoParserReturnValue, *DemoParserReturnValueRef;
#endif
/**
 * Type holding context data for http_parser_t functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct DemoParser_s;
typedef struct DemoParser_s DemoParser, *DemoParserRef;
typedef void(*DP_MessageComple_CB)(void* ctx, DemoMessageRef, int error_code);
struct DemoParser_s {
    ParserInterface;
    DECLARE_TAG;
    int  m_state;
    void* on_read_ctx;
    DP_MessageComple_CB on_message_complete;
//    void(*on_read_message_cb)(void* read_ctx, DemoMessageRef msg, int error_code);
//    void(*on_read_parser_error_cb)(void* read_ctx, const char* error_message);
    DemoMessageRef  m_current_message_ptr;
};

DemoParserRef DemoParser_new(
        DP_MessageComple_CB on_message_complete_cb,
        void* on_read_ctx);
void DemoParser_dispose(DemoParserRef* parser_p);
void DemoParser_free(DemoParserRef this);

DemoParserErrCode DemoParser_consume(DemoParserRef parser, IOBufferRef iobuffer_ref);

int  DemoParser_get_errno(DemoParserRef parser);
DemoParserError     DemoParser_get_error(DemoParserRef parser);

#endif


/**@}*/