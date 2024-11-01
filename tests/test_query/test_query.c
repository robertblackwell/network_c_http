
#include <http_in_c/common/alloc.h>
#include <rbl/unittest.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

int test_query ()
{
    char str[80] = "path1=1&path2=222&path3";
    char* str_char = str;
    void* str_void = (void*) str;
    const char s[2] = "&";
    char *token;

    /* get the first token */
    token = strtok(str, s);

    /* walk through other tokens */
    while( token != NULL ) {
        printf( " %s\n", token );

        token = strtok(NULL, s);
    }

    return(0);
}

int main()
{
    UT_ADD(test_query);
    int rc = UT_RUN();
    return rc;
}