#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <c_eg/alloc.h>
#include <c_eg/utils.h>
char* make_upper(char* src)
{
    int srclen = strlen(src);
    char* dest = eg_alloc(srclen+1);
    char* s = src;
    char* p = dest;
    for(int i = 0; i < srclen; i++) {
        p[i] = toupper((unsigned char) src[i]);
    }
    p[srclen] = (unsigned char)'\0';
    return dest;
}
