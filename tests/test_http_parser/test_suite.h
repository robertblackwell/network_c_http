
#include "helper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// EQ011  request with error no minor version
///////////////////////////////////////////////////////////////////////////////////////////////////////////
static int vfunc_eq011 (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    UT_EQUAL_INT(rref->rc, READER_PARSE_ERROR);
    return 0;
//    UT_EQUAL_CSTR(Message_get_reason(m1), "OK 11Reason Phrase");
#ifdef A_ON
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ContentLength).get() == "11");
    CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
    return 0;
#endif
}

ParserTestRef test_case_a0011() {
// A0011
    char *description = "A0011 parser error no minor version";
    char *lines[] = {
            (char *) "GET /target HTTP/1.\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 0\r\n\r\n",
            NULL
    };
    return ParserTest_new(description, lines, vfunc_eq011);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// EQ012  request simulated IO error
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_eq012_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    UT_EQUAL_INT(rref->rc, READER_IO_ERROR);
    return 0;
//    UT_EQUAL_CSTR(Message_get_reason(m1), "OK 11Reason Phrase");
#ifdef A_ON
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ContentLength).get() == "11");
    CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
    return 0;
#endif
}
ParserTestRef test_case_eq012() {
    char *description = "A0012 simulated io error";
    char *lines[] = {
            (char *) "GET /target HTTP/1.1\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "error",
            NULL
    };
    return ParserTest_new(description, lines, test_eq012_vfunc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK001  response OK 200 with body and content length 10
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// A001
int test_ROK001_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    UT_EQUAL_INT(Message_get_status (m1), 200);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "11");
    BufferChainRef body = HttpMessage_get_body (m1);
    bool x = BufferChain_eq_cstr (body, "01234567890");
    CHECK_BODY(m1, "01234567890");
    return 0;
}
ParserTestRef test_case_ROK001() {
    static char *description = "A001 response 200 with body and content length 10";
    static char *lines[] = {
            (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 11\r\n\r\n",
            (char *) "01234567890",
            NULL
    };
    return ParserTest_new(description, lines, test_ROK001_vfunc);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK002  response OK 201 with body and content length 10 some body data in header buffer
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK002_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 22Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "ABCDEFGHIJ");

    return 0;
}
//A002
ParserTestRef test_case_ROK002() {
    static char *description = "ROK002 response 201 body length 10 SOME body data in header buffer";
    static char *lines[] = {
            (char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 10\r\n\r\nAB",
            (char *) "CDEFGHIJ",
            NULL
    };
    return ParserTest_new(description, lines, test_ROK002_vfunc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK003  response OK 201 with body and content length 10 data in same buffer as black header line
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK003_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 22Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "ABCDEFGHIJ");

    KVPairRef hlref_host = HdrList_find (h, HEADER_HOST);
    UT_EQUAL_CSTR(KVPair_label (hlref_host), HEADER_HOST);
    UT_EQUAL_CSTR(KVPair_value (hlref_host), "ahost");

    KVPairRef hlref_connection = HdrList_find (h, HEADER_CONNECTION);
    UT_EQUAL_CSTR(KVPair_label (hlref_connection), HEADER_CONNECTION);
    UT_EQUAL_CSTR(KVPair_value (hlref_connection), "keep-alive");

    KVPairRef hlref_proxy_connection = HdrList_find (h, HEADER_PROXYCONNECTION);
    UT_EQUAL_CSTR(KVPair_label (hlref_proxy_connection), HEADER_PROXYCONNECTION);
    UT_EQUAL_CSTR(KVPair_value (hlref_proxy_connection), "keep-alive");

    KVPairRef hlref_content_length = HdrList_find (h, HEADER_CONTENT_LENGTH);
    UT_EQUAL_CSTR(KVPair_label (hlref_content_length), HEADER_CONTENT_LENGTH);
    UT_EQUAL_CSTR(KVPair_value (hlref_content_length), "10");
    return 0;
};

// A003
ParserTestRef test_case_ROK003()
{
    static char *description = "ROK003 response 201 body length 10 SOME body data in with blank line buffer EOH and EOM at same time";
    static char *lines[] = {
            (char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 10",
            (char *) "\r\n\r\nABCDEFGHIJ",
            NULL
    };
    return ParserTest_new(description, lines, test_ROK003_vfunc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK004  response OK 200 with body and content length 10
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK004_vfunc (ListRef results) {
    ReadResultRef rref = (ReadResultRef) List_remove_first(results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist(m1);
    UT_EQUAL_INT(Message_get_status(m1), 201);
    UT_EQUAL_CSTR(Message_get_reason(m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");
    return 0;
}
// A004
ParserTestRef test_case_ROK004() {
    static char *description = "ROK004 response 201 body chunked encoding NO body data in header buffer";
    static char *lines[] = {
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
    return ParserTest_new(description, lines, test_ROK004_vfunc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK005  response OK 201 chunked encoding - some body data in buffer with blank header line
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK005_vfunc (ListRef results);

// A005
ParserTestRef test_case_ROK005() {
    static char *description = "ROK005 response  201 body chunked encoding SOME body data in buffer with blank line after header";
    static char *lines[] = {
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
    return ParserTest_new(description, lines, test_ROK005_vfunc);
}
int test_ROK005_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");

    return 0;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK006  response OK 201 simple chunked body
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK006_vfunc (ListRef results);

// A006
ParserTestRef test_case_ROK006() {
    static char *description = "ROK006 simple 201 body chunked - chunks spread over different buffers";
    static char *lines[] = {
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
    return ParserTest_new(description, lines, test_ROK006_vfunc);
}
int test_ROK006_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    UT_EQUAL_INT(Message_get_status (m1), 201);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK Reason Phrase");

    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_TRANSFERENCODING, "chunked");
    CHECK_BODY(m1, "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");

    return 0;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK007  request and response back to back
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK007_vfunc (ListRef results);

// A007
ParserTestRef test_case_ROK007()
{
    static char *description = "ROK007 request and response back to back ";
    static char *lines[] = {
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
    return ParserTest_new(description, lines, test_ROK007_vfunc);
}
int test_ROK007_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    ReadResultRef rref2 = (ReadResultRef) List_remove_first (results);
    MessageRef m2 = rref2->message;
    UT_NOT_EQUAL_PTR(m1, m2);
    UT_NOT_EQUAL_PTR(m1, NULL);
    UT_NOT_EQUAL_PTR(m2, NULL);
    HdrListRef h1 = Message_get_headerlist (m1);
    HdrListRef h2 = Message_get_headerlist (m2);
    UT_NOT_EQUAL_PTR(h1, h2);
    UT_NOT_EQUAL_PTR(h1, NULL);
    UT_NOT_EQUAL_PTR(h2, NULL);
    {
        HdrListRef h = Message_get_headerlist (m1);
        UT_EQUAL_INT(Message_get_status (m1), 200);
        UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

        CHECK_HEADER(h, HEADER_HOST, "ahost");
        CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
        BufferChainRef bcref = HttpMessage_get_body (m1);
        CHECK_BODY(m1, "1234567890");
    }
    {
        HdrListRef h = Message_get_headerlist (m2);
        UT_EQUAL_INT(Message_get_status (m2), 201);
        UT_EQUAL_CSTR(Message_get_reason (m2), "OK 22Reason Phrase");

        CHECK_HEADER(h, HEADER_HOST, "ahost");
        CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
        CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "11");

        BufferChainRef bcref = HttpMessage_get_body(m2);
        IOBufferRef iobref = BufferChain_compact(bcref);
        bool x01 = BufferChain_eq_cstr(bcref, "ABCDEFGHIJK");
        int y = x01;
        CHECK_BODY(m2, "ABCDEFGHIJK");
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK008  response OK 200 no content length - finish with EOF
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int test_ROK008_vfunc (ListRef results);
ParserTestRef test_case_ROK008() {
// A008
    static char *description = "ROK008 No content-length, has transfer encoding but not chunked. Parsing should make content-length: 10";
    static char *lines[] = {
            (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
            (char *) "Host: ahost\r\n",
            (char *) "Transfer-encoding: dummy\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep\0    ",
            (char *) "-alive\r\n\0    ",
            (char *) "\r\n",
            (char *) "1234567890",
            (char *) NULL,
    };
    return ParserTest_new(description, lines, test_ROK008_vfunc);
}
int test_ROK008_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    int n = HdrList_size (h);
    UT_EQUAL_INT(Message_get_status (m1), 200);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

    KVPairRef hlr = HdrList_find (h, HEADER_HOST);
    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "1234567890");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ROK009  response OK 200 no content length no transfer encoding
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// A009
int test_ROK009_vfunc (ListRef results);
ParserTestRef test_case_ROK009() {
    static char *description = "ROK009 No content-length, no transfer encoding. Parses correctly";
    static char *lines[] = {
            (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep\0    ",
            (char *) "-alive\r\n\0    ",
            (char *) "\r\n",
            (char *) "1234567890",
            (char *) NULL,
    };
    return ParserTest_new(description, lines, test_ROK009_vfunc);
}
int test_ROK009_vfunc (ListRef results)
{
    ReadResultRef rref = (ReadResultRef) List_remove_first (results);
    MessageRef m1 = rref->message;
    HdrListRef h = Message_get_headerlist (m1);
    int n = HdrList_size (h);
    UT_EQUAL_INT(Message_get_status (m1), 200);
    UT_EQUAL_CSTR(Message_get_reason (m1), "OK 11Reason Phrase");

    KVPairRef hlr = HdrList_find (h, HEADER_HOST);
    CHECK_HEADER(h, HEADER_HOST, "ahost");
    CHECK_HEADER(h, HEADER_CONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_PROXYCONNECTION, "keep-alive");
    CHECK_HEADER(h, HEADER_CONTENT_LENGTH, "10");
    CHECK_BODY(m1, "1234567890");
    return 0;
}

#if 0
ListRef make_test_A ()
{
    ListRef tl = List_new ();
    List_add_back (tl, test_case_ROK001());
    List_add_back (tl, test_case_ROK002());
    List_add_back (tl, test_case_ROK003());
    List_add_back (tl, test_case_ROK004());
    List_add_back (tl, test_case_ROK005());
    List_add_back (tl, test_case_ROK006());
    List_add_back (tl, test_case_ROK007());
    List_add_back (tl, test_case_ROK008());
    List_add_back (tl, test_case_ROK009());
    return tl;
}

ListRef make_test_B ()
{
    ListRef tl = List_new (NULL);
    List_add_back (tl, test_case_ROK001());
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
        ParserTestRef x = (ParserTestRef) List_itr_unpack (tests, iter);
        DataSource source;
        DataSource_init (&source, x->lines);
        WrappedParserTest wpt;
        printf ("Running %s\n", x->description);
        WPT_init (&wpt, &source, x->verify_function);
        result = result || WPT_run (&wpt);
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

int main ()
{
    printf("sizeof http_parser: %ld,  sizeof http_parser_settings: %ld\n", sizeof(llhttp_t), sizeof(llhttp_settings_t));
//    UT_ADD(test_A);
    UT_ADD(test_B);
    int rc = UT_RUN();
    return rc;
}
#endif
