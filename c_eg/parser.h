
#ifndef c_eg_parser_h
#define c_eg_parser_h
/**
 * This modules wraps the c language http parser provided by github.com/joyent/http-parser
 * for use in this project.
 */
#include <http-parser/http_parser.h>
#include <c_eg/message.h>
#include <c_eg/buffer/cbuffer.h>

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
    enum http_errno     m_err_number;
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
typedef struct Parser_s Parser;

struct Parser_s {
    bool m_started;
    bool m_header_done;
    bool m_message_done;
    /*
     * These are required to run the http-parser
     */
    http_parser*             m_http_parser_ptr;
    http_parser_settings*    m_http_parser_settings_ptr;
    Message*               m_current_message_ptr;

    int                      m_header_state;
    ///////////////////////////////////////////////////////////////////////////////////
    // String buffers used to accumulate values from http-parser
    ///////////////////////////////////////////////////////////////////////////////////
    Cbuffer*             m_url_buf;
    Cbuffer*             m_status_buf;
    Cbuffer*             m_name_buf;
    Cbuffer*             m_value_buf;
};

Parser* Parser_new();
void Parser_free(Parser** parser_p);

void Parser_begin(Parser* parser, Message* msg_ref);

/**
 * The guts of the http message parsing process.
 *
 * Call this function repeatedly with successive buffers of data.
 * THese successive buffers may represent one or more messages and a single buffer is permitted to hold
 * the end of one message and the start of the successive message.
 *
 * The returned value indicates the state the parser is in after processing a buffer, and under some circumstances
 * how much of the provided buffer was consumed.
 *
 * Except under error conditions, the only time a buffer will be only partially consumed is when
 * a messages ends part way through a buffer. This is indicated by message complete being returned
 * along with number of bytes consumed being less that the size of the buffer provided.
 *
 * Under such situation the completed message should be retreived from the parser, the parser begin()
 * method called again and the remainder of the incomplete buffer presented to the parser to start
 * the next message.
 *
 * \param parser Parser*
 * \param buffer A buffer of data presumable read from a tcp connectin
 * \param length Length of the data ba=uffer
 * \return ParserReturnValue - a struct
 */
ParserReturnValue Parser_consume(Parser* parser, const void* buffer, int length);

/**
 * Returns the message currently being worked on. Only valid after Parser_consume() returns ParserReturnValue.return_code == ReturnRC_end_of_message
 * \param parser Parser*
 * \return Message* or NULL
 */
Message*      Parser_current_message(Parser* parser);

/**
 * Gather details of latest error
 * \param parser
 * \return
 */
bool            Parser_is_error(Parser* parser);
enum http_errno Parser_get_errno(Parser* parser);
ParserError     Parser_get_error(Parser* parser);

/**
 * C parser class callback functions that interface with the C language parser
 * http-parser from github.com/joyent/http-parser.
 *
 * These could have been hidden in the parser.c file
*/
int message_begin_cb(http_parser* parser);
int url_data_cb(http_parser* parser, const char* at, size_t length);
int status_data_cb(http_parser* parser, const char* at, size_t length);
int header_field_data_cb(http_parser* parser, const char* at, size_t length);
int header_value_data_cb(http_parser* parser, const char* at, size_t length);
int headers_complete_cb(http_parser* parser);
int chunk_header_cb(http_parser* parser);
int body_data_cb(http_parser* parser, const char* at, size_t length);
int chunk_complete_cb(http_parser* parser);
int message_complete_cb(http_parser* parser);

#endif


