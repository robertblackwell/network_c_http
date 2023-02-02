
#ifndef c_http_ll_parser_types_h
#define c_http_ll_parser_types_h
#include <stdint.h>
#include <llhttp/llhttp.h>
/**
 * @addtogroup group_parser_types
 * @{
 */
// from http-parser
typedef uint16_t      HttpStatus;
typedef llhttp_method_t HttpMethod;
typedef llhttp_errno_t  HttpErrno;


/* Status Codes */
#define C_HTTP_STATUS_MAP(C_HTTP_XX)                                                 \
  C_HTTP_XX(100, CONTINUE,                        Continue)                        \
  C_HTTP_XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  C_HTTP_XX(102, PROCESSING,                      Processing)                      \
  C_HTTP_XX(200, OK,                              OK)                              \
  C_HTTP_XX(201, CREATED,                         Created)                         \
  C_HTTP_XX(202, ACCEPTED,                        Accepted)                        \
  C_HTTP_XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  C_HTTP_XX(204, NO_CONTENT,                      No Content)                      \
  C_HTTP_XX(205, RESET_CONTENT,                   Reset Content)                   \
  C_HTTP_XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  C_HTTP_XX(207, MULTI_STATUS,                    Multi-Status)                    \
  C_HTTP_XX(208, ALREADY_REPORTED,                Already Reported)                \
  C_HTTP_XX(226, IM_USED,                         IM Used)                         \
  C_HTTP_XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  C_HTTP_XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  C_HTTP_XX(302, FOUND,                           Found)                           \
  C_HTTP_XX(303, SEE_OTHER,                       See Other)                       \
  C_HTTP_XX(304, NOT_MODIFIED,                    Not Modified)                    \
  C_HTTP_XX(305, USE_PROXY,                       Use Proxy)                       \
  C_HTTP_XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  C_HTTP_XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  C_HTTP_XX(400, BAD_REQUEST,                     Bad Request)                     \
  C_HTTP_XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  C_HTTP_XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  C_HTTP_XX(403, FORBIDDEN,                       Forbidden)                       \
  C_HTTP_XX(404, NOT_FOUND,                       Not Found)                       \
  C_HTTP_XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  C_HTTP_XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  C_HTTP_XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  C_HTTP_XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  C_HTTP_XX(409, CONFLICT,                        Conflict)                        \
  C_HTTP_XX(410, GONE,                            Gone)                            \
  C_HTTP_XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  C_HTTP_XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  C_HTTP_XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  C_HTTP_XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  C_HTTP_XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  C_HTTP_XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  C_HTTP_XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  C_HTTP_XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  C_HTTP_XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  C_HTTP_XX(423, LOCKED,                          Locked)                          \
  C_HTTP_XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  C_HTTP_XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  C_HTTP_XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  C_HTTP_XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  C_HTTP_XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  C_HTTP_XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  C_HTTP_XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  C_HTTP_XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  C_HTTP_XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  C_HTTP_XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  C_HTTP_XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  C_HTTP_XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  C_HTTP_XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  C_HTTP_XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  C_HTTP_XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  C_HTTP_XX(510, NOT_EXTENDED,                    Not Extended)                    \
  C_HTTP_XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

enum http_status
  {
#define C_HTTP_XX(num, name, string) Z_HTTP_STATUS_##name = num,
  C_HTTP_STATUS_MAP(C_HTTP_XX)
#undef C_HTTP_XX
  };

const char *
http_status_str (enum http_status s);

/** @} */
#endif


