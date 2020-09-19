#include "./test_set.h>"

#undefine A_ON
// A001    
    char* test_A001_description = "response 200 with body and content length 10",
    char* test_A001_lines[] = {
        (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
        (char *) "Host: ahost\r\n",
        (char *) "Connection: keep-alive\r\n",
        (char *) "Proxy-Connection: keep-alive\r\n",
        (char *) "Content-length: 11\r\n\r\n",
        (char *) "01234567890",
        NULL
    },
    void test_A001_vfunc(MessageRef  messages[])
    {
        MessageRef m1 = messages[0];
        HeaderFields& h = m1->headers();
        auto x = m1->status_code();
#ifdef A_ON
        CHECK((m1->status_code() == 200));
        CHECK((m1->reason() == "OK 11Reason Phrase"));
        CHECK(h.at_key(HeaderFields::Host));
        CHECK(h.at_key(HeaderFields::Connection));
        CHECK(h.at_key(HeaderFields::ProxyConnection));
        CHECK(h.at_key(HeaderFields::ContentLength));
        CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
        CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
        CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
        CHECK(h.at_key(HeaderFields::ContentLength).get() == "11");
        CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
#endif
    }

//A002
        char* test_A002_description = "response 201 body length 10 SOME body data in header buffer",
        char* test_A002_lines[] = {
            (char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 10\r\n\r\nAB",
            (char *) "CDEFGHIJ",
            NULL
        }
        void test_A002_vfunc(MessageRef messages[])
        {
            MessageBase::SPtr m1 = messages[0];
            HeaderFields& h = m1->headers();
            auto x = m1->status_code();
#ifdef A_ON
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
        }

// A003
        char* test_A003_description = "response 201 body length 10 SOME body data in with blank line buffer EOH and EOM at same time";
        char* test_A003_lines[] = {
            (char *) "HTTP/1.1 201 OK 22Reason Phrase\r\n",
            (char *) "Host: ahost\r\n",
            (char *) "Connection: keep-alive\r\n",
            (char *) "Proxy-Connection: keep-alive\r\n",
            (char *) "Content-length: 10",
            (char *) "\r\n\r\nABCDEFGHIJ",
            NULL
        };
        void Test_A003_vfunc(MessageRef messages[])
        {
            MessageRef m1 = messages[0];
            HeaderFields& h = m1->headers();
#ifdef A_ON
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
        };


// A004
        char* test_A004_description = "response 201 body chunked encoding NO body data in header buffer";
        char* test_A004_lines[] = {
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
        void test_A004_vfun(MessageRef messages[])
        {
            MessageBase::SPtr m1 = messages[0];
            HeaderFields& h = m1->headers();
    #ifdef A_ON
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
        }
// A005
        char* test_A005_description = "response  201 body chunked encoding SOME body data in buffer with blank line after header";
        char* test_A005_lines[] = {
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
        void test_A005_vfunc(MessageRef messages[])
        {
            MessageRef m1 = messages[0];
            HeaderFields& h = m1->headers();
    #ifdef A_ON
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

        }
// A006
        test_A006_description = "simple 201 body chunked - chunks spread over different buffers",
        char* test_A006_lines[] = {
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
        }
        void test_A006_vfunc(MessageBase messages[])
        {
            MessageBase::SPtr m1 = messages[0];
            HeaderFields& h = m1->headers();
    #ifdef A_ON
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

        }
// A007
        char* test_A007_description = "request and response back to back "; 
        char* test_A007_lines[] = {
            (char*) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
            (char*) "Host: ahost\r\n",
            (char*) "Connection: keep-alive\r\n",
            (char*) "Proxy-Connection: keep\0    ",
            (char*) "-alive\r\n\0    ",
            (char*) "Content-length: 10\r\n\r\n",
            (char*) "1234567",
            (char*) "890HTTP/1.1 ",
            (char*) "201 OK 22Reason Phrase\r\n",
            (char*) "Host: ahost\r\n",
            (char*) "Connection: keep-alive\r\n",
            (char*) "Proxy-Connection: keep-alive\r\n",
            (char*) "Content-length: 11\r\n",
            (char*) "\r\n",
            (char*) "ABCDEFGHIJK\0      ",
            NULL
            };
        void test_A007_vfunc(MessageRef messages[])
        {
            REQUIRE(messages.size() > 0);
    #ifdef A_ON
            MessageRef m0 = messages[0];
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
        }

// A008
    char* test_A008_description = "";
    char* test_A008_lines[] = {
        (char*) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
        (char*) "Host: ahost\r\n",
        (char*) "Connection: keep-alive\r\n",
        (char*) "Proxy-Connection: keep\0    ",
        (char*) "-alive\r\n\0    ",
        (char*) "\r\n",
        (char*) "1234567890",
        NULL
    void test_A008_vfunc(MessageRef messages[])
    {
        MessageRef m0 = messages[0];
        HeaderFields& h = m0->headers();
    #ifdef A_ON
        CHECK(m0->version_major() == 1);
        CHECK(m0->version_minor() == 1);
        CHECK(m0->status_code() == 200);
        CHECK(h.at_key("CONNECTION").get() == "keep-alive");
        CHECK(h.at_key("PROXY-CONNECTION").get() == "keep-alive");
        auto b0 = m0->get_body_buffer_chain()->to_string();
        CHECK(m0->get_body()->to_string() == "1234567890");
#endif

    }


TestSet test_set_A[] = {
    {test_A001_description, test_A001_lines, test_A001_vfunc},
    {test_A002_description, test_A002_lines, test_A002_vfunc},
    {test_A003_description, test_A003_lines, test_A003_vfunc},
    {test_A004_description, test_A004_lines, test_A004_vfunc},
    {test_A005_description, test_A005_lines, test_A005_vfunc},
    {test_A006_description, test_A006_lines, test_A006_vfunc},
    {test_A007_description, test_A007_lines, test_A007_vfunc},
    {test_A008_description, test_A008_lines, test_A008_vfunc},
}