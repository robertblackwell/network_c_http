/**
 * @file message
 * @brief   Module for parsing http/1.1 messages. It does this by providing a wrapper for
 *          github.com/nodejs/llhttp which replaces github.com/joyent/http-parser
 */


#ifndef c_http_ll_parser_h
#define c_http_ll_parser_h
#include <stdint.h>
#include <llhttp/llhttp.h>
#include <c_http/details/ll_parser_types.h>
#include <c_http/api/message.h>
#include <c_http/dsl/cbuffer.h>
/**
 * @addtogroup http
 * @{
 */

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
    llhttp_errno_t      m_err_number;
};
typedef struct ParserError_s ParserError;

/**
 * \brief  Return code used as part of the value returned by Parser_consume() when processing data.
 */
enum ParserRC {
    ParserRC_error,          /// got a parse error
    ParserRC_end_of_header,  /// encountered enf of header
    ParserRC_end_of_message, /// encountered end of message
    ParserRC_end_of_data     /// processed all the data given
};
typedef enum ParserRC ParserRC;

/**
 * Value object return by Parser_consume()
 */
struct ParserReturnValue {
    ParserRC   return_code;
    int        bytes_remaining;
};

typedef struct ParserReturnValue ParserReturnValue;

/**
 * Type holding context data for Parser functions. Allows for parsing to continue
 * over buffer and message boundaries
 */
struct Parser_s;
typedef struct Parser_s Parser, *ParserRef;

struct Parser_s {
    bool m_started;
    bool m_header_done;
    bool m_message_done;
    /*
     * These are required to run the http-parser
     */
    llhttp_t*                m_llhttp_ptr;
    llhttp_settings_t*       m_llhttp_settings_ptr;
    MessageRef               m_current_message_ptr;

    int                      m_header_state;
    ///////////////////////////////////////////////////////////////////////////////////
    // String buffers used to accumulate values from http-parser
    ///////////////////////////////////////////////////////////////////////////////////
    CbufferRef             m_url_buf;
    CbufferRef             m_status_buf;
    CbufferRef             m_name_buf;
    CbufferRef             m_value_buf;
};

ParserRef Parser_new();
void Parser_free(ParserRef* parser_p);

void Parser_begin(ParserRef parser, MessageRef msg_ref);

/**
 * @brief The guts of the http message parsing process.
 *
 * Call this function repeatedly with successive buffers of data.
 * These successive buffers may represent one or more messages and a single buffer is permitted to hold
 * the end of one message and the start of the successive message.
 *
 * The returned value indicates the state the parser is in after processing a buffer, and under some circumstances
 * how much of the provided buffer was consumed.
 *
 * Except under error conditions, the only time a buffer will be only partially consumed is when
 * a messages ends part way through a buffer. This is indicated by message complete true being returned
 * along with number of bytes consumed being less that the size of the buffer provided.
 *
 * Under such situation the completed message should be retrieved from the parser, the parser begin()
 * method called again and the remainder of the incomplete buffer presented to the parser to start
 * the next message.
 *
 * @param parser ParserRef
 * @param buffer A buffer of data presumable read from a tcp connectin
 * @param length Length of the data ba=uffer
 * @return ParserReturnValue - a struct
 */
ParserReturnValue Parser_consume(ParserRef parser, const void* buffer, int length);

/**
 * @brief Returns the message currently being worked on. Only valid after Parser_consume() returns ParserReturnValue.return_code == ReturnRC_end_of_message
 * @param parser ParserRef
 * @return MessageRef or NULL
 */
MessageRef      Parser_current_message(ParserRef parser);

llhttp_errno_t  Parser_get_errno(ParserRef parser);
ParserError     Parser_get_error(ParserRef parser);

#endif


/**@}*/