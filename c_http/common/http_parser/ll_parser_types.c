#include <c_http/common/http_parser/ll_parser_types.h>

const char *
http_status_str (enum http_status s)
{
  switch (s) {
#define C_HTTP_XX(num, name, string) case HTTP_STATUS_##name: return #string;
    C_HTTP_STATUS_MAP(C_HTTP_XX)
#undef C_HTTP_XX
    default: return "<unknown>";
  }
}



