#ifndef c_c_eg_parser_test_data_eof_h
#define c_c_eg_parser_test_data_eof_h

//
// 001
//
    char* test_001_description = "Body with No content length and header line broken into 2 buffers";
    char* test_001_lines[] = {
        (char*) "HTTP/1.1 200 OK 11Reason Phrase\r\n\0        ",
        (char*) "Host: ahost\r\n",
        (char*) "Connection: keep-alive\r\n",
        (char*) "Proxy-Connection: keep\0    ",
        (char*) "-alive\r\n\0    ",
        (char*) "\r\n",
        (char*) "1234567890",
        NULL
    },

    void test_001_vfunc(MessageRef messages[])
    {
        MessageRef m0 = messages[0];
        CHECK(m0->version_major() == 1);
        CHECK(m0->version_minor() == 1);
        CHECK(m0->status_code() == 200);
        
        HeaderFields& h = m0->headers();

        CHECK(h.at_key("CONNECTION").get() == "keep-alive");
        CHECK(h.at_key("PROXY-CONNECTION").get() == "keep-alive");
        auto b0 = m0->get_body_buffer_chain()->to_string();
        CHECK(m0->get_body()->to_string() == "1234567890");

    }

// 002
    char* test_002_description = "Body with no content length header and explicit eof"
    char* test_002_lines[] = {
        (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
        (char *) "Host: ahost\r\n",
        (char *) "Connection: keep-alive\r\n",
        (char *) "Proxy-Connection: keep-alive\r\n",
        (char *) "\r\n",
        (char *) "01234567890",
        (char *) "eof",
        NULL
    },
    void test_002_vfunc(MessageRef messages[])
    {
        MessageRef m1 = messages[0];
        auto x = m1->status_code();
        CHECK((m1->status_code() == 200));
        CHECK((m1->reason() == "OK 11Reason Phrase"));
        HeaderFields& h = m1->headers();
        CHECK( (!!h.at_key(HeaderFields::Host)));
        CHECK( (!!h.at_key(HeaderFields::Connection)));
        CHECK( (!!h.at_key(HeaderFields::ProxyConnection)));
        CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
        CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
        CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
        CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
    }
// 003
    char* test_003_description = "Body with no content length header and explicit close"
    char* test_003_lines[] = {
        (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
        (char *) "Host: ahost\r\n",
        (char *) "Connection: keep-alive\r\n",
        (char *) "Proxy-Connection: keep-alive\r\n",
        (char *) "\r\n",
        (char *) "01234567890",
        (char *) "close",
        NULL
    },
    void test_003_vfunc(MessageRef  messages[])
    {
        MessageRef m1 = messages[0];
        auto x = m1->status_code();
        CHECK((m1->status_code() == 200));
        CHECK((m1->reason() == "OK 11Reason Phrase"));
        HeaderFields& h = m1->headers();
        CHECK(h.at_key(HeaderFields::Host));
        CHECK(h.at_key(HeaderFields::Connection));
        CHECK(h.at_key(HeaderFields::ProxyConnection));
        CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
        CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive");
        CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
        CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
    }
// 004    
    };
    char* test_004_description = "Body, no content length explicit eof one header with multi values"
    char* test_004_lines[] = {
        (char *) "HTTP/1.1 200 OK 11Reason Phrase\r\n",
        (char *) "Connection: keep-alive , TE, somethingelse\r\n",
        (char *) "Host: ahost\r\n",
        (char *) "Proxy-Connection: keep-alive\r\n",
        (char *) "\r\n",
        (char *) "01234567890",
        (char *) "eof",
        NULL
    },
    test_004_vfunc(MessageRef  messages[])
    {
        MessageBase::SPtr m1 = messages[0];
        HeaderFields& h = m1->headers();
        auto x = m1->status_code();
        CHECK((m1->status_code() == 200));
        CHECK((m1->reason() == "OK 11Reason Phrase"));
        CHECK(h.at_key(HeaderFields::Host));
        CHECK(h.at_key(HeaderFields::Connection));
        CHECK(h.at_key(HeaderFields::ProxyConnection));
        CHECK(h.at_key(HeaderFields::Host).get() == "ahost");
        CHECK(h.at_key(HeaderFields::Connection).get() == "keep-alive , TE, somethingelse");
        CHECK(h.at_key(HeaderFields::ProxyConnection).get() == "keep-alive");
        CHECK(m1->get_body_buffer_chain()->to_string() == "01234567890");
    }

#endif