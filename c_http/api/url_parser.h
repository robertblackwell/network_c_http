#ifndef c_http_url_parser_h
#define c_http_url_parser_h
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#if defined(_WIN32) && !defined(__MINGW32__) && \
  (!defined(_MSC_VER) || _MSC_VER<1600) && !defined(__WINE__)
#include <BaseTsd.h>
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

/* Compile with -DHTTP_PARSER_STRICT=0 to make less checks, but run
 * faster
 */
#ifndef C_HTTP_URL_PARSER_STRICT
# define C_HTTP_URL_PARSER_STRICT 1
#endif

/* Map for errno-related constants
 *
 * The provided argument should be a macro that takes 2 arguments.
 */
#define C_HTTP_URL_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Callback-related errors */                                      \
  XX(CB_message_begin, "the on_message_begin callback failed")       \
  XX(CB_url, "the on_url callback failed")                           \
  XX(CB_header_field, "the on_header_field callback failed")         \
  XX(CB_header_value, "the on_header_value callback failed")         \
  XX(CB_headers_complete, "the on_headers_complete callback failed") \
  XX(CB_body, "the on_body callback failed")                         \
  XX(CB_message_complete, "the on_message_complete callback failed") \
  XX(CB_status, "the on_status callback failed")                     \
  XX(CB_chunk_header, "the on_chunk_header callback failed")         \
  XX(CB_chunk_complete, "the on_chunk_complete callback failed")     \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(UNEXPECTED_CONTENT_LENGTH,                                      \
     "unexpected content-length header")                             \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")                           \
  XX(INVALID_TRANSFER_ENCODING,                                      \
     "request has invalid transfer-encoding")                        \


/* Define UPE_* values for each errno value above */
#define C_HTTP_URL_ERRNO_GEN(n, s) UPE_##n,
enum c_http_url_errno {
    C_HTTP_URL_ERRNO_MAP(C_HTTP_URL_ERRNO_GEN)
};
#undef C_HTTP_URL_ERRNO_GEN

enum c_http_parser_url_fields
{ UF_SCHEMA           = 0
    , UF_HOST             = 1
    , UF_PORT             = 2
    , UF_PATH             = 3
    , UF_QUERY            = 4
    , UF_FRAGMENT         = 5
    , UF_USERINFO         = 6
    , UF_MAX              = 7
};


/* Result structure for http_parser_parse_url().
 *
 * Callers should index into field_data[] with UF_* values iff field_set
 * has the relevant (1 << UF_*) bit set. As a courtesy to clients (and
 * because we probably have padding left over), we convert any port to
 * a uint16_t.
 */
struct c_http_parser_url {
    uint16_t field_set;           /* Bitmask of (1 << UF_*) values */
    uint16_t port;                /* Converted UF_PORT string */

    struct {
        uint16_t off;               /* Offset into buffer in which field starts */
        uint16_t len;               /* Length of run in buffer */
    } field_data[UF_MAX];
};


/* Return a string name of the given error */
const char *c_http_errno_name(enum http_errno err);

/* Return a string description of the given error */
const char *c_http_errno_description(enum c_http_url_errno err);

/* Initialize all http_parser_url members to 0 */
void c_http_parser_url_init(struct c_http_parser_url *u);

/* Parse a URL; return nonzero on failure */
int c_http_parser_parse_url(const char *buf, size_t buflen,
                          int is_connect,
                          struct c_http_parser_url *u);


#ifdef __cplusplus
}
#endif
#endif
