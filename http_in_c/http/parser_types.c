#include <http_in_c/http/parser_types.h>

const char * http_status_str (HttpStatus s)
{
    return llhttp_status_name(s);
}



