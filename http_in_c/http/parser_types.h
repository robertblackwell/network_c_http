
#ifndef c_http_ll_parser_types_h
#define c_http_ll_parser_types_h
#include <stdint.h>
#include <llhttp/llhttp.h>
typedef llhttp_status_t HttpStatus;
typedef llhttp_method_t HttpMethod;
typedef llhttp_errno_t  HttpErrno;

const char *
http_status_str (HttpStatus s);

/** @} */
#endif


