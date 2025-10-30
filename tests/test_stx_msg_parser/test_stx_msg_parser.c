

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <src/http/kvpair.h>
#include <apps/msg/stx/stx_msg.h>
#include "test_harness.h"
#include "run_list.c"

/////////////////////////////////////////////////////////////////////////////////////////
// B001
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B001 (ListRef results)
{
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    UT_TRUE((rref->message != NULL))
    StxMsgRef m1 = rref->message;
    int rc = rref->rc;
    IOBufferRef cb = stx_msg_get_body(rref->message);
    IOBufferRef bb = stx_msg_serialize(m1);
    const char* x = IOBuffer_cstr(cb);
    return 0;
}
static parser_test_t* test_case_B001() {
    static const char *description = "B001 simple single buffer message";
    static const char* lines[] = {
            (char *) "\x02QQ" "1234567890\x03",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B001);
    return ptc;
}
/////////////////////////////////////////////////////////////////////////////////////////
// B002
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B002 (ListRef results)
{
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    UT_TRUE((rref->message != NULL))
    StxMsgRef m1 = rref->message;
    int rc = rref->rc;
    IOBufferRef cb = stx_msg_get_body(rref->message);
    IOBufferRef bb = stx_msg_serialize(m1);
    const char* x = IOBuffer_cstr(cb);
    return 0;
}
static parser_test_t* test_case_B002() {
    static const char *description = "B002 simple multi buffer message";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345",
            (char *) "67890\x03X",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B001);
    return ptc;
}
/////////////////////////////////////////////////////////////////////////////////////////
// B003
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B003 (ListRef results)
{
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    UT_TRUE((rref->message != NULL))
    StxMsgRef m1 = rref->message;
    int rc = rref->rc;
    IOBufferRef cb = stx_msg_get_body(rref->message);
    IOBufferRef bb = stx_msg_serialize(m1);
    const char* x = IOBuffer_cstr(cb);
    return 0;
}
static parser_test_t* test_case_B003() {
    static const char *description = "B003 rubbish before stx correct etx ending";
    static const char* lines[] = {
            (char *) "AAA" "\x02QQ" "12345678900987654321\x03X",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B003);
    return ptc;
}
/////////////////////////////////////////////////////////////////////////////////////////
// B004
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B004 (ListRef results)
{
    UT_TRUE((List_size(results) == 0))
    return 0;
}
static parser_test_t* test_case_B004() {
    static const char *description = "B004 EOF and no etx - will not find a message";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345678900987654321",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B004);
    return ptc;
}
/////////////////////////////////////////////////////////////////////////////////////////
// B005
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B005 (ListRef results)
{
    int sz = List_size(results);
    UT_TRUE((List_size(results) == 1))
    test_output_r rref = (test_output_r) List_remove_first (results);
    StxMsgRef m1 = rref->message;
    UT_TRUE((m1 != NULL))
    int rc1 = rref->rc;
    return 0;
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
/////////////////////////////////////////////////////////////////////////////////////////
// B006
/////////////////////////////////////////////////////////////////////////////////////////
int vfunc_B006 (ListRef results)
{
    int sz = List_size(results);
    UT_TRUE((List_size(results) == 2))
    test_output_r rref_1 = (test_output_r) List_remove_first (results);
    test_output_r rref_2 = (test_output_r) List_remove_first (results);
    StxMsgRef m1 = rref_1->message;
    StxMsgRef m2 = rref_2->message;
    UT_TRUE((m1 != NULL))
    IOBufferRef iob1 = stx_msg_get_content(m1);
    UT_TRUE((strcmp(IOBuffer_cstr(iob1), "QQ12345678") == 0))
    UT_TRUE((m2 != NULL))
    IOBufferRef iob2 = stx_msg_get_content(m2);
    UT_TRUE((strcmp(IOBuffer_cstr(iob2), "QQ987654321") == 0))

    int rc1 = rref_1->rc;
    return 0;
}
static parser_test_t* test_case_B006() {
    static const char *description = "B006 multiple good message in one buffer ";
    static const char* lines[] = {
            (char *) "\x02QQ" "12345678" "\x03" "\x02QQ" "987654321" "\03",
            (char *) NULL,
    };
    parser_test_r ptc = parser_test_new(description, lines, vfunc_B006);
    return ptc;
}

ListRef make_demo_test()
{
//    parser_test_r test_X001 = parser_test_new(test_B001_description, test_B001_lines, test_B001_vfunc);
//    parser_test_r test_X002 = parser_test_new(test_B002_description, test_B002_lines, test_B001_vfunc);
    ListRef tl = List_new ();
    List_add_back (tl, test_case_B001());
    List_add_back (tl, test_case_B002());
    List_add_back (tl, test_case_B003());
    List_add_back (tl, test_case_B004());
    List_add_back (tl, test_case_B005());
    List_add_back (tl, test_case_B006());
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