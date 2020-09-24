
#ifndef c_eg_parser_h
#define c_eg_parser_h
/**
 * This modules wraps the c language http parser provided by github.com/joyent/http-parser
 * for use in this project.
 */
#include <http-parser/http_parser.h>
#include <c_eg/message.h>
#include <c_eg/buffer/contig_buffer.h>

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
typedef struct Parser_s Parser, *ParserRef;

struct Parser_s {
    bool m_started;
    bool m_header_done;
    bool m_message_done;
    /*
     * These are required to run the http-parser
     */
    http_parser*             m_http_parser_ptr;
    http_parser_settings*    m_http_parser_settings_ptr;
    MessageRef               m_current_message_ptr;

    int                      m_header_state;
    ///////////////////////////////////////////////////////////////////////////////////
    // String buffers used to accumulate values from http-parser
    ///////////////////////////////////////////////////////////////////////////////////
    CBufferRef             m_url_buf;
    CBufferRef             m_status_buf;
    CBufferRef             m_name_buf;
    CBufferRef             m_value_buf;
};

ParserRef Parser_new();
void Parser_free(ParserRef* parser_p);

void Parser_begin(ParserRef parser, MessageRef msg_ref);
/**
 * The guts of the parsing process. Call this function repeatedly with successive buffers of data.
 * The returned value will indicate error, message complete, need more data or error paring latest buffer
 *
 * \param parser ParserRef
 * \param buffer A buffer of data presumable read from a tcp connectin
 * \param length Length of the data ba=uffer
 * \return ParserReturnValue - a struct
 */
ParserReturnValue Parser_consume(ParserRef parser, const void* buffer, int length);

/**
 * Returns the message currently being worked on. Only valid after Parser_consume() returns ParserReturnValue.return_code == ReturnRC_end_of_message
 * \param parser ParserRef
 * \return MessageRef or NULL
 */
MessageRef      Parser_current_message(ParserRef parser);

/**
 * Gather details of latest error
 * \param parser
 * \return
 */
bool            Parser_is_error(ParserRef parser);
enum http_errno Parser_get_errno(ParserRef parser);
ParserError     Parser_get_error(ParserRef parser);

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


