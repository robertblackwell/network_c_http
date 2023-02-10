

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <http_in_c/unittest.h>
#include <http_in_c/logger.h>
#include <http_in_c/http/kvpair.h>
#include <http_in_c/common/http_parser/datasource.h>
#include <http_in_c/demo_protocol/demo_parser_test.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_sync_reader_private.h>
#include <http_in_c/demo_protocol/demo_sync_reader.h>


// B001
char *test_B001_description = "B001 simple single buffer message";
char *test_B001_lines[] = {
        (char *) "\x01Q\x02" "1234567890\x03X",
        (char *) NULL,
};
// B002
char *test_B002_description = "B002 same as B001 but over 2 buffers";
char *test_B002_lines[] = {
        (char *) "\x01Q\x02" "1",
        (char *) "234567890\x03X",
        (char *) NULL,
};

int test_B001_vfunc (ListRef results)
{
    DemoReadResultRef rref = (DemoReadResultRef) List_remove_first (results);
    DemoMessageRef m1 = rref->message;
    BufferChainRef b = demo_message_get_body(rref->message);
    IOBufferRef cb = BufferChain_compact(b);
    IOBufferRef bb = demo_message_serialize(m1);
    const char* x = IOBuffer_cstr(cb);
    return 0;
}
ListRef make_demo_test()
{
    DemoParserTestRef test_X001 = DemoParserTest_new (test_B001_description, test_B001_lines, test_B001_vfunc);
    DemoParserTestRef test_X002 = DemoParserTest_new (test_B002_description, test_B002_lines, test_B001_vfunc);
    ListRef tl = List_new (NULL);
    List_add_back (tl, test_X002);
    return tl;

}

int run_list (ListRef tests)
{
    int result = 0;
    // note all labels are upper case
    ListIterator iter = List_iterator (tests);
    for (;;) {
        if (iter == NULL) {
            break;
        }
        ListIterator next = List_itr_next (tests, iter);
        DemoParserTestRef x = (DemoParserTestRef) List_itr_unpack (tests, iter);
        DataSource source;
        DataSource_init (&source, x->lines);
        DemoWrappedParserTest wpt;
        printf ("Running %s\n", x->description);
        DemoWPT_init (&wpt, &source, x->verify_function);
        result = result || DemoWPT_run (&wpt);
        iter = next;
    }
    return result;
}
int test_demo_1()
{
    return run_list(make_demo_test());
}

int main ()
{
    printf("sizeof http_parser: %ld,  sizeof http_parser_settings: %ld\n", sizeof(llhttp_t), sizeof(llhttp_settings_t));
    UT_ADD(test_demo_1);
    int rc = UT_RUN();
    return rc;
}