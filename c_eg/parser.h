
#ifndef c_eg_parser_h
#define c_eg_parser_h

#include <http-parser/http_parser.h>
#include <c_eg/message.h>
#include <c_eg/buffer/contig_buffer.h>

#define kHEADER_STATE_NOTHING 10
#define kHEADER_STATE_FIELD   11
#define kHEADER_STATE_VALUE   12

struct ParserError_s {
    char*               m_name;
    char*               m_description;
    enum http_errno     m_err_number;
};
typedef struct ParserError_s ParserError;
/**
 * \brief  Value returned by parse when processing data.
 */
enum ParserRC {
    ParserRC_error,          /// got a parse error
    ParserRC_end_of_header,  /// encountered enf of header
    ParserRC_end_of_message, /// encountered end of message
    ParserRC_end_of_data     /// processed all the data given
};
typedef enum ParserRC ParserRC;

struct ParserReturnValue {
    ParserRC   return_code;
    int        bytes_remaining;
};
typedef struct ParserReturnValue ParserReturnValue;
struct Parser_s;
typedef struct Parser_s Parser, *ParserRef;

struct Parser_s {
    bool m_started;
    bool m_header_done;
    bool m_message_done;
    /*
     * These are required to run the parser
     */
    http_parser*             m_http_parser_ptr;
    http_parser_settings*    m_http_parser_settings_ptr;
    MessageRef               m_current_message_ptr;
//    BodyBufferStrategy       m_buffer_strategy;
//    ContigBufferFactoryT     m_factory;

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

//int  Parser_append_bytes(ParserRef parser, void* buffer,  unsigned length);
void Parser_begin(ParserRef parser, MessageRef msg_ref);
ParserReturnValue Parser_consume(ParserRef parser, const void* buffer, int length);
#ifdef NOIMPLEMENTED

//ParserReturnValue Parser_end(ParserRef parser);
//void Parser_append_eof(ParserRef parser);
#endif

bool            Parser_is_error(ParserRef parser);
enum http_errno Parser_get_errno(ParserRef parser);
ParserError     Parser_get_error(ParserRef parser);
MessageRef      Parser_current_message(ParserRef parser);

/**
* C parser class callback functions that interface with the C language parser
* http-parser from nodejs
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


