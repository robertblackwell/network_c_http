

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <http_in_c/http/kvpair.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include "test_harness.h"
#include "run_list.c"
// B002
char *test_B002_description = "B002 same as B001 but over 2 buffers";
char *test_B002_lines[] = {
        (char *) "\x01Q\x02" "1",
        (char *) "234567890\x03X",
        (char *) NULL,
};

int vfunc_B001 (ListRef results)
{
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    UT_TRUE((rref->message != NULL))
    DemoMessageRef m1 = rref->message;
    int rc = rref->rc;
    BufferChainRef b = demo_message_get_body(rref->message);
    IOBufferRef cb = BufferChain_compact(b);
    IOBufferRef bb = demo_message_serialize(m1);
    const char* x = IOBuffer_cstr(cb);
    return 0;
}
int vfunc_B004 (ListRef results)
{
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    DemoMessageRef m1 = rref->message;
    UT_TRUE((m1 == NULL))
    int rc = rref->rc;
    UT_TRUE((rc == DemoParserErr_expected_stx))
    return 0;
}

int vfunc_B005 (ListRef results)
{
    int sz = List_size(results);
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    DemoMessageRef m1 = rref->message;
    UT_TRUE((m1 != NULL))
    int rc1 = rref->rc;
    return 0;
}


//// B001
static parser_test_t* test_case_B001() {
    static const char *description = "B001 simple single buffer message";
    static const char* lines[] = {
            (char *) "\x02QQ" "1234567890\x03",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B001);
    return ptc;
}
static parser_test_t* test_case_B002() {
    static const char *description = "B002 simple single buffer message";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345",
            (char *) "67890\x03X",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B001);
    return ptc;
}
static parser_test_t* test_case_B003() {
    static const char *description = "B003 rubish before stx no etx";
    static const char* lines[] = {
            (char *) "AAA" "\x02QQ" "12345678900987654321X",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B004);
    return ptc;
}
static parser_test_t* test_case_B004() {
    static const char *description = "B004 EOF and no etx";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345678900987654321",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B004);
    return ptc;
}
static parser_test_t* test_case_B005() {
    static const char *description = "B005 good message followed by rubbishrubbish after etx ";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345678" "\x03" "900987654321",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B005);
    return ptc;
}

ListRef make_demo_test()
{
//    parser_test_r test_X001 = parser_test_new(test_B001_description, test_B001_lines, test_B001_vfunc);
//    parser_test_r test_X002 = parser_test_new(test_B002_description, test_B002_lines, test_B001_vfunc);
    ListRef tl = List_new (NULL);
//    List_add_back (tl, test_case_B001());
//    List_add_back (tl, test_case_B002());
//    List_add_back (tl, test_case_B004());
//    List_add_back (tl, test_case_B003());
    List_add_back (tl, test_case_B005());
//    List_add_back (tl, test_case_B005());
    return tl;
}

int test_demo_1()
{
    return run_list(make_demo_test());
}

int main ()
{
    UT_ADD(test_demo_1);
    int rc = UT_RUN();
    return rc;
}