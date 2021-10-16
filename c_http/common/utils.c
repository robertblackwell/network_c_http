#include <string.h>
#include <ctype.h>

#include <c_http/common/alloc.h>
#include <c_http/common/utils.h>
char* make_upper(const char* src)
{
    int srclen = strlen(src);
    char* dest = eg_alloc(srclen+1);
    char* p = dest;
    for(int i = 0; i < srclen; i++) {
        p[i] = toupper((unsigned char) src[i]);
    }
    p[srclen] = (unsigned char)'\0';
    return dest;
}
