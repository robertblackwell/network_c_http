#ifndef demo_parser_h
#define demo_parser_h
#include <stdint.h>
#include <c_http/demo_protocol/demo_message.h>
#include <c_http/common/cbuffer.h>
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
 * \brief  Return code used as part of the value returned by Parser_consume() when processing data.
 */
enum DemoParserRC {
    DemoParserRC_end_of_message = 0x01,
    DemoParserRC_message_incomplete = 0x00,
    DemoParserRC_error = -1,
//    DemoParserRC_invalid_opcode,
//    DemoParserRC_expected_stx,
//    DemoParserRC_expected_ascii,
};
typedef int DemoParseErrCode;
#define DemoParserErr_invalid_opcode -11
#define DemoParserErr_expected_stx   -12
#define DemoParserErr_expected_ascii -13

typedef enum DemoParserRC DemoParserRC;

/**
 * Value object return by Parser_consume()
 */
struct DemoParserReturnValue {
    long            bytes_consumed;
    bool            eom_flag;
    int             error_code;
};

typedef struct DemoParserReturnValue DemoParserReturnValue;

/**
 * Type holding context data for Parser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct DemoParser_s;
typedef struct DemoParser_s DemoParser, *DemoParserRef;

struct DemoParser_s {
    bool m_started;
    bool m_message_done;
    int  m_state;
    DemoMessageRef  m_current_message_ptr;
    ///////////////////////////////////////////////////////////////////////////////////
    // String buffers used to accumulate values demo-parser
    ///////////////////////////////////////////////////////////////////////////////////
    CbufferRef             m_opcode_buf;
};

DemoParserRef DemoParser_new();
void DemoParser_dispose(DemoParserRef* parser_p);

void DemoParser_begin(DemoParserRef parser, DemoMessageRef msg_ref);

DemoParserReturnValue DemoParser_comsume_iobuffer(DemoParserRef parser_ref, IOBufferRef iobuf);
DemoParserReturnValue DemoParser_consume(DemoParserRef parser, const void* buffer, int length);

/**
 * @brief Returns the message currently being worked on. Only valid after Parser_consume() returns ParserReturnValue.return_code == ReturnRC_end_of_message
 * @param parser ParserRef
 * @return MessageRef or NULL
 */
DemoMessageRef      DemoParser_current_message(DemoParserRef parser);

int  DemoParser_get_errno(DemoParserRef parser);
DemoParserError     DemoParser_get_error(DemoParserRef parser);

#endif


/**@}*/