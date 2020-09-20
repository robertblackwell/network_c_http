#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <c_eg/unittest.h>
#include <c_eg/logger.h>
#include <c_eg/header_line.h>
#include <c_eg/test_helper_types.h>
#include <c_eg/message.h>

#undef A_ON
#define CHECK_HEADER(h, K, V) do {\
    HeaderLineRef hlref = HDRList_find(h, HEADER_HOST); \
    UT_NOT_EQUAL_PTR(hlref, NULL); \
    UT_EQUAL_CSTR(HeaderLine_label(hlref), HEADER_HOST); \
    UT_EQUAL_CSTR(HeaderLine_value(hlref), "ahost"); \
} while(0);

#define CHECK_BODY(M, S) do {\
    BufferChainRef body = Message_get_body(M); \
    bool x##M = BufferChain_eq_cstr(body, S); \
    UT_EQUAL_INT(x##M, 1); \
} while(0);

// A001
char *test_A001_description = "A001 response 200 with body and content length 10";
char *test_A001_lines[] = {
(char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Content-length: 11\r\n\r\n",
(char *) "01234567890",
NULL
};

int test_A001_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 200);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "11");
    BufferChainRef body = Message_get_body (m1);
    bool x = BufferChain_eq_cstr (body, "01234567890");
    CHECK_BODY(m1, "01234567890");
    return 0;
}

//A002
char *test_A002_description = "A002 response 201 body length 10 SOME body data in header buffer";
char *test_A002_lines[] = {
(char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Content-length: 10\r\n\r\nAB",
(char *) "CDEFGHIJ",
NULL
};

int test_A002_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 22Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "ABCDEFGHIJ");

    return 0;
}

// A003
char *test_A003_description = "A003 response 201 body length 10 SOME body data in with blank line buffer EOH and EOM at same time";
char *test_A003_lines[] = {
(char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Content-length: 10",
(char *) "\r\n\r\nABCDEFGHIJ",
NULL
};

int test_A003_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 22Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "ABCDEFGHIJ");

    HeaderLineRef hlref_host = HDRList_find (h, HEADER_HOST);
    UT_EQUAL_CSTR(HeaderLine_label (hlref_host), HEADER_HOST);
    UT_EQUAL_CSTR(HeaderLine_value (hlref_host), "ahost");

    HeaderLineRef hlref_connection = HDRList_find (h, HEADER_CONNECTION);
    UT_EQUAL_CSTR(HeaderLine_label (hlref_connection), HEADER_CONNECTION);
    UT_EQUAL_CSTR(HeaderLine_value (hlref_connection), "keep-alive");

    HeaderLineRef hlref_proxy_connection = HDRList_find (h, HEADER_PROXYCONNECTION);
    UT_EQUAL_CSTR(HeaderLine_label (hlref_proxy_connection), HEADER_PROXYCONNECTION);
    UT_EQUAL_CSTR(HeaderLine_value (hlref_proxy_connection), "keep-alive");

    HeaderLineRef hlref_content_length = HDRList_find (h, HEADER_CONTENT_LENGTH);
    UT_EQUAL_CSTR(HeaderLine_label (hlref_content_length), HEADER_CONTENT_LENGTH);
    UT_EQUAL_CSTR(HeaderLine_value (hlref_content_length), "10");

    return 0;

};


// A004
char *test_A004_description = "A004 response 201 body chunked encoding NO body data in header buffer";
char *test_A004_lines[] = {
(char *) "HTTP/1.1 201 OK Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Transfer-Encoding: chunked\r\n\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0f\r\n1234567890XXXXX\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0f\r\n1234567890HGHGH\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0\r\n",
(char *) "\r\n",
NULL
};

int test_A004_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");

    return 0;

}

// A005
char *test_A005_description = "A005 response  201 body chunked encoding SOME body data in buffer with blank line after header";
char *test_A005_lines[] = {
(char *) "HTTP/1.1 201 OK Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Transfer-Encoding: chunked\r\n",
(char *) "\r\n0a\r\n1234567890\r\n",
(char *) "0f\r\n1234567890XXXXX\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0f\r\n1234567890HGHGH\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0\r\n",
(char *) "\r\n",
NULL
};

int test_A005_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");

    return 0;

}

// A006
char *test_A006_description = "A006 simple 201 body chunked - chunks spread over different buffers";
char *test_A006_lines[] = {
(char *) "HTTP/1.1 201 OK Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Transfer-Encoding: chunked\r\n",
(char *) "\r\n0a\r\n123456",
(char *) "7890\r\n",
(char *) "0f\r\n123456",
(char *) "7890XXXXX\r\n0a\r\n1234567890\r\n",
(char *) "0f\r\n1234567890HGHGH\r\n",
(char *) "0a\r\n1234567890\r\n",
(char *) "0\r\n",
(char *) "\r\n",
NULL
};

int test_A006_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");

    return 0;

}

// A007
char *test_A007_description = "A007 request and response back to back ";
char *test_A007_lines[] = {
(char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep\0    ",
(char *) "-alive\r\n\0    ",
(char *) "Content-length: 10\r\n\r\n",
(char *) "1234567",
(char *) "890HTTP/1.1 ",
(char *) "201 OK 22Reason Phrase\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Content-length: 11\r\n",
(char *) "\r\n",
(char *) "ABCDEFGHIJK\0      ",
NULL
};

int test_A007_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    MessageRef m2 = (MessageRef) List_remove_first (messages);
    UT_NOT_EQUAL_PTR(m1, m2);
    UT_NOT_EQUAL_PTR(m1, NULL);
    UT_NOT_EQUAL_PTR(m2, NULL);
    HDRListRef h1 = Message_headers (m1);
    HDRListRef h2 = Message_headers (m2);
    UT_NOT_EQUAL_PTR(h1, h2);
    UT_NOT_EQUAL_PTR(h1, NULL);
    UT_NOT_EQUAL_PTR(h2, NULL);
    {
        HDRListRef h = Message_headers (m1);
        UT_EQUAL_INT(Message_get_status (m1), 200);
        UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

        CHECK_HEADER(h, HEADER_HOST, "ahost");
        CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
        BufferChainRef bcref = Message_get_body (m1);
        CHECK_BODY(m1, "1234567890");
    }
    {
        HDRListRef h = Message_headers (m2);
        UT_EQUAL_INT(Message_get_status (m2), 201);
        UT_EQUAL_CSTR(Message_get_reason (m2), "OK 22Reason Phrase");

        CHECK_HEADER(h, HEADER_HOST, "ahost");
        CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "11");

        BufferChainRef bcref = Message_get_body(m2);
        CBufferRef cbref = BufferChain_compact(bcref);
        bool x01 = BufferChain_eq_cstr(bcref, "ABCDEFGHIJK");
        int y = x01;
        CHECK_BODY(m2, "ABCDEFGHIJK");
    }
    return 0;
}

// A008
char *test_A008_description = "A008 No content-length";
char *test_A008_lines[] = {
(char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep\0    ",
(char *) "-alive\r\n\0    ",
(char *) "\r\n",
(char *) "1234567890",
(char *) NULL,
};

int test_A008_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HDRListRef h = Message_headers (m1);
    int n = HDRList_size (h);
    UT_EQUAL_INT(Message_get_status (m1), 200);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

    HeaderLineRef hlr = HDRList_find (h, HEADER_HOST);
    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "1234567890");
    return 0;
}

ListRef make_test_A ()
{
    ParserTestRef test_A001 = ParserTest_new (test_A001_description, test_A001_lines, test_A001_vfunc);
    ParserTestRef test_A002 = ParserTest_new (test_A002_description, test_A002_lines, test_A002_vfunc);
    ParserTestRef test_A003 = ParserTest_new (test_A003_description, test_A003_lines, test_A003_vfunc);
    ParserTestRef test_A004 = ParserTest_new (test_A004_description, test_A004_lines, test_A004_vfunc);
    ParserTestRef test_A005 = ParserTest_new (test_A005_description, test_A005_lines, test_A005_vfunc);
    ParserTestRef test_A006 = ParserTest_new (test_A006_description, test_A006_lines, test_A006_vfunc);
    ParserTestRef test_A007 = ParserTest_new (test_A007_description, test_A007_lines, test_A007_vfunc);
    ParserTestRef test_A008 = ParserTest_new (test_A008_description, test_A008_lines, test_A008_vfunc);
    ListRef tl = List_new (NULL);
    List_add_back (tl, test_A001);
    List_add_back (tl, test_A002);
    List_add_back (tl, test_A003);
    List_add_back (tl, test_A004);
    List_add_back (tl, test_A005);
    List_add_back (tl, test_A006);
    List_add_back (tl, test_A008); // still a problem with this one
    List_add_back(tl, test_A007);
    return tl;
}

ListRef make_test_B ()
{
    ParserTestRef test_A008 = ParserTest_new (test_A008_description, test_A008_lines, test_A008_vfunc);
    ParserTestRef test_A007 = ParserTest_new (test_A007_description, test_A007_lines, test_A007_vfunc);
    ListRef tl = List_new (NULL);
    // there is a bug in this one
    List_add_back (tl, test_A008);
//    List_add_back (tl, test_A007);
//    List_add_back (tl, test_A007);
//    List_add_back (tl, test_A007);
//    List_add_back (tl, test_A007);
//    List_add_back (tl, test_A007);
//    List_add_back (tl, test_A007);
    return tl;
}

int run_list (ListRef tests)
{
    int result = 0;
    // note all labels are upper case
    ListNodeRef iter = List_iterator (tests);
    for (;;) {
        if (iter == NULL) {
            break;
        }
        ListNodeRef next = List_itr_next (tests, iter);
        ParserTestRef x = (ParserTestRef) List_itr_unpack (tests, iter);
        DataSource source;
        DataSource_init (&source, x->lines);
        WrappedParserTest wpt;
        ParserRef parser = Parser_new ();
        printf ("Running %s\n", x->description);
        WPT_init (&wpt, parser, &source, x->verify_function);
        result = result || WPT_run2 (&wpt);
        iter = next;
    }
    return result;}
int test_A()
{
    return run_list(make_test_A());
}
int test_B()
{
    return run_list(make_test_B());
}
///////////////////////////////////////////////////
int test_01 ()
{
    ListRef tests = make_test_A ();
    // note all labels are upper case
    ListNodeRef iter = List_iterator (tests);
    for (;;) {
        if (iter == NULL) {
            break;
        }
        ListNodeRef next = List_itr_next (tests, iter);
        ParserTestRef x = (ParserTestRef) List_itr_unpack (tests, iter);
        DataSource source;
        DataSource_init (&source, x->lines);
        for (;;) {
            char buffer[1000];
            char *b = &buffer[0];
            int buffer_length = 1000;
            int bytes_read = DataSource_read (&source, buffer, buffer_length);
            if (bytes_read == 0) {
                break;
            }
        }
        iter = next;
    }
    return 0;
}

int test_02 ()
{
    int result = 0;
    ListRef tests = make_test_B ();
    // note all labels are upper case
    ListNodeRef iter = List_iterator (tests);
    for (;;) {
        if (iter == NULL) {
            break;
        }
        ListNodeRef next = List_itr_next (tests, iter);
        ParserTestRef x = (ParserTestRef) List_itr_unpack (tests, iter);
        DataSource source;
        DataSource_init (&source, x->lines);
        WrappedParserTest wpt;
        ParserRef parser = Parser_new ();
        printf ("Running %s", x->description);
        WPT_init (&wpt, parser, &source, x->verify_function);
        result = result || WPT_run2 (&wpt);
        iter = next;
    }
    return result;
}

int main ()
{
    UT_ADD(test_A);
//    UT_ADD(test_B);
    int rc = UT_RUN();
    return rc;
}