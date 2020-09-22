#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <c_eg/unittest.h>
#include <c_eg/logger.h>
#include <c_eg/header_line.h>

///////////////////////////////////////////////////
int test_header_line_new()
{
    char* lab = "Content-length"; int lablen = strlen(lab);
    char* val = "123456"; int vallen = strlen(val);
    HeaderLineRef hlref = HeaderLine_new(lab, lablen, val, vallen);
    char* l = HeaderLine_label(hlref);
    int ll = strlen(l);
    char* v = HeaderLine_value(hlref);
    int vl = strlen(v);
    // note all labels are upper case
    int lb = strcmp(l, "CONTENT-LENGTH");
    UT_EQUAL_INT(lb, 0);
    int vb = strcmp(v, val);
    UT_EQUAL_INT(vb, 0);
    char* val2 = "98989898989";
    HeaderLine_set_value(hlref, val2, strlen(val2));
    char* v2 = HeaderLine_value(hlref);
    int v2b = strcmp(v2, val2);
    UT_EQUAL_INT(v2b, 0);
    // note all labels are upper case
    HeaderLine_free(&hlref);
    return 0;
}

int main()
{
    UT_ADD(test_header_line_new);
    int rc = UT_RUN();
    return rc;
}