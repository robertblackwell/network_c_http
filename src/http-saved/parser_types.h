
#ifndef c_http_ll_parser_types_h
#define c_http_ll_parser_types_h
#include <stdint.h>
#include <http-parser-2.9.4/http_parser.h>
typedef enum http_status HttpStatus;
typedef enum http_method HttpMethod;
typedef enum http_errno  HttpErrno;
const char *
http_status_str (HttpStatus s);

/** @} */
#endif


