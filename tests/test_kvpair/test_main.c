#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <c_http/unittest.h>
#include <c_http/logger.h>
#include <c_http/kvpair.h>

///////////////////////////////////////////////////
int test_kvpair_new()
{
    char* lab = "Content-length"; int lablen = strlen(lab);
    char* val = "123456"; int vallen = strlen(val);
    KVPairRef hlref = KVPair_new(lab, lablen, val, vallen);
    char* l = KVPair_label(hlref);
    int ll = strlen(l);
    char* v = KVPair_value(hlref);
    int vl = strlen(v);
    // note all labels are upper case
    int lb = strcmp(l, "CONTENT-LENGTH");
    UT_EQUAL_INT(lb, 0);
    int vb = strcmp(v, val);
    UT_EQUAL_INT(vb, 0);
    char* val2 = "98989898989";
    KVPair_set_value(hlref, val2, strlen(val2));
    char* v2 = KVPair_value(hlref);
    int v2b = strcmp(v2, val2);
    UT_EQUAL_INT(v2b, 0);
    // note all labels are upper case
    KVPair_free(&hlref);
    return 0;
}

int main()
{
    UT_ADD(test_kvpair_new);
    int rc = UT_RUN();
    return rc;
}