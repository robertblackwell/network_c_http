#include <http_in_c/unittest.h>
#include <http_in_c/http/header_list.h>
#include <http_in_c/http/message.h>

#undef A_ON
// A001
char* test_A001_description = "response 200 with body and content length 10";
char* test_A001_lines[] = {
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
    HdrListRef h = Message_headers(m1);
    int x = Message_get_status(m1);
    UT_EQUAL_INT(Message_get_status(m1), 200);
    UT_EQUAL_CSTR(Message_get_reason(m1), "OK 11Reason Phrase");
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

// A0011
char* test_A0011_description = "response 200 with body and content length 10";
char* test_A0011_lines[] = {
(char *) "GET /target HTTP/1.1\r\n",
(char *) "Host: ahost\r\n",
(char *) "Connection: keep-alive\r\n",
(char *) "Proxy-Connection: keep-alive\r\n",
(char *) "Content-length: 0\r\n\r\n",
NULL
};

int test_A0011_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
    HdrListRef h = Message_headers(m1);
    int x = Message_get_status(m1);
    UT_EQUAL_INT(Message_get_method(m1), HttpMethod);
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


//A002
char *test_A002_description = "response 201 body length 10 SOME body data in header buffer";
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
#ifdef A_ON
    auto x = m1->status_code();
    HeaderFields& h = m1->headers();
    CHECK((m1->status_code() == 201));
    CHECK((m1->reason() == "OK 22Reason Phrase"));
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ContentLength).get() == "10");
    CHECK(m1->get_body_buffer_chain()->to_string() == "ABCDEFGHIJ");
#endif
    return 0;
}

// A003
char *test_A003_description = "response 201 body length 10 SOME body data in with blank line buffer EOH and EOM at same time";
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
#ifdef A_ON
    HeaderFields& h = m1->headers();
    auto x = m1->status_code();
    CHECK((m1->status_code() == 201));
    CHECK((m1->reason() == "OK 22Reason Phrase"));
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ContentLength).get() == "10");
    CHECK(m1->get_body_buffer_chain()->to_string() == "ABCDEFGHIJ");
#endif
    return 0;

};


// A004
char *test_A004_description = "response 201 body chunked encoding NO body data in header buffer";
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
#ifdef A_ON
    HeaderFields& h = m1->headers();
    auto x = h.at_key(HeaderFields::TransferEncoding);
    CHECK((m1->status_code() == 201));
    CHECK((m1->reason() == "OK Reason Phrase"));
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(!h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::TransferEncoding));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::TransferEncoding).get() == "chunked");
    CHECK(m1->get_body_buffer_chain()->to_string()
              == "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");
#endif
    return 0;

}

// A005
char *test_A005_description = "response  201 body chunked encoding SOME body data in buffer with blank line after header";
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
#ifdef A_ON
    HeaderFields& h = m1->headers();
    auto x = h.at_key(HeaderFields::TransferEncoding);
    CHECK((m1->status_code() == 201));
    CHECK((m1->reason() == "OK Reason Phrase"));
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(!h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::TransferEncoding));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::TransferEncoding).get() == "chunked");
    CHECK(m1->get_body_buffer_chain()->to_string()
              == "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");
#endif
    return 0;

}

// A006
char *test_A006_description = "simple 201 body chunked - chunks spread over different buffers";
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
#ifdef A_ON
    HeaderFields& h = m1->headers();
    auto x = h.at_key(HeaderFields::TransferEncoding);
    CHECK((m1->status_code() == 201));
    CHECK((m1->reason() == "OK Reason Phrase"));
    CHECK(h.at_key(HeaderFields::Host));
    CHECK(h.at_key(HeaderFields::Connection));
    CHECK(h.at_key(HeaderFields::ProxyConnection));
    CHECK(!h.at_key(HeaderFields::ContentLength));
    CHECK(h.at_key(HeaderFields::TransferEncoding));
    CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
    CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
    CHECK(h.at_key(HeaderFields::TransferEncoding).get() == "chunked");
    CHECK(m1->get_body_buffer_chain()->to_string()
              == "12345678901234567890XXXXX12345678901234567890HGHGH1234567890");
#endif
    return 0;

}

// A007
char *test_A007_description = "request and response back to back ";
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
#ifdef A_ON
    HeaderFields& h = m0->headers();
    REQUIRE(m0 != nullptr);
    CHECK(m0->version_major() == 1);
    CHECK(m0->version_minor() == 1);
    CHECK(m0->status_code() == 200);
    CHECK(h.at_key("CONTENT-LENGTH").get() == "10");
    CHECK(h.at_key("CONNECTION").get() == "keep-alive");
    CHECK(h.at_key("PROXY-CONNECTION").get() == "keep-alive");
    auto b0 = m0->get_body_buffer_chain()->to_string();
    CHECK(m0->get_body()->to_string() == "1234567890");
    MessageRef m1 = messages[1];
    HeaderFields& h2 = m1->headers();
    REQUIRE(m1 != nullptr);
    CHECK(m1->version_major() == 1);
    CHECK(m1->version_minor() == 1);
    CHECK(m1->status_code() == 201);
    CHECK(h2.at_key("CONTENT-LENGTH").get() == "11");
    CHECK(h2.at_key("CONNECTION").get() == "keep-alive");
    CHECK(h2.at_key("PROXY-CONNECTION").get() == "keep-alive");
    auto b1 = m1->get_body_buffer_chain()->to_string();
    CHECK(m1->get_body()->to_string() == "ABCDEFGHIJK");
#endif
    return 0;
}

// A008
char *test_A008_description = "";
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

void test_A008_vfunc (ListRef messages)
{
    MessageRef m1 = (MessageRef) List_remove_first (messages);
#ifdef A_ON
    HeaderFields& h = m0->headers();
    CHECK(m0->version_major() == 1);
    CHECK(m0->version_minor() == 1);
    CHECK(m0->status_code() == 200);
    CHECK(h.at_key("CONNECTION").get() == "keep-alive");
    CHECK(h.at_key("PROXY-CONNECTION").get() == "keep-alive");
    auto b0 = m0->get_body_buffer_chain()->to_string();
    CHECK(m0->get_body()->to_string() == "1234567890");
#endif

}

ListRef make_test_A()
{
    ParserTestRef test_A0011 = ParserTest_new(test_A0011_description, test_A0011_lines, test_A0011_vfunc);
    ParserTestRef test_A001 = ParserTest_new(test_A001_description, test_A001_lines, test_A001_vfunc);
    ParserTestRef test_A002 = ParserTest_new(test_A002_description, test_A002_lines, test_A002_vfunc);
    ParserTestRef test_A003 = ParserTest_new(test_A003_description, test_A003_lines, test_A003_vfunc);
    ParserTestRef test_A004 = ParserTest_new(test_A004_description, test_A004_lines, test_A004_vfunc);
    ParserTestRef test_A005 = ParserTest_new(test_A005_description, test_A005_lines, test_A005_vfunc);
    ParserTestRef test_A006 = ParserTest_new(test_A006_description, test_A006_lines, test_A006_vfunc);
    ParserTestRef test_A007 = ParserTest_new(test_A007_description, test_A007_lines, test_A007_vfunc);
    ParserTestRef test_A008 = ParserTest_new(test_A008_description, test_A008_lines, test_A008_vfunc);
    ListRef tl = List_new(NULL);
    List_add_back(tl, test_A0011);
    List_add_back(tl, test_A001);
    List_add_back(tl, test_A002);
    List_add_back(tl, test_A003);
    List_add_back(tl, test_A004);
    List_add_back(tl, test_A005);
    List_add_back(tl, test_A006);
    List_add_back(tl, test_A007);
    List_add_back(tl, test_A008);
    return tl;
}
