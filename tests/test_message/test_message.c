
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <http_in_c/common/cbuffer.h>
#include <rbl/logger.h>
#include <http_in_c/common/list.h>
#include <http_in_c/http/kvpair.h>
#include <http_in_c/http/header_list.h>
#include <http_in_c/http/http_message.h>


#ifdef HDRXX
///////////////////////////////////////////////////
int test_hdrlist_new()
{
    HdrListRef hdrlistref = HdrList_new();
    int sz = HdrList_size(hdrlistref);
    UT_NOT_EQUAL_PTR(hdrlistref, NULL);
    UT_EQUAL_INT(sz, 0);
    HdrList_dispose(&hdrlistref);
    UT_EQUAL_PTR(hdrlistref, NULL);
	return 0;
}
int test_hdrlist_add_back_get_content()
{
    HdrListRef hdrlistref = HdrList_new();
    KVPairRef hdrln1 = KVPair_new("KVPairKey1", strlen("KVPairKey1"), "333", strlen("333"));
    KVPairRef hdrln2 = KVPair_new("KVPairKey2", strlen("KVPairKey2"), "4444", strlen("4444"));
    HdrList_add_back(hdrlistref, hdrln1);
    HdrList_add_back(hdrlistref, hdrln2);
    int sz = HdrList_size(hdrlistref);
    UT_EQUAL_INT(sz, 2);
    KVPairRef hdrref1 = HdrList_first(hdrlistref);
    char* sh1 = KVPair_label(hdrref1);
    char* sv1 = KVPair_value(hdrref1);
    KVPairRef hdrref2 = HdrList_last(hdrlistref);
    char* sh2 = KVPair_label(hdrref2);
    char* sv2 = KVPair_value(hdrref2);
    UT_EQUAL_INT(strcmp(sh1, "KVPAIRKEY1"), 0);
    UT_EQUAL_INT(strcmp(sv1, "333"), 0);
    UT_EQUAL_INT(strcmp(sh2, "KVPAIRKEY2"), 0);
    UT_EQUAL_INT(strcmp(sv2, "4444"), 0);
    List_display((ListRef)hdrlistref);
    HdrList_dispose(&hdrlistref);
    return 0;
}
int test_hdrlist_find()
{
    HdrListRef hdrlistref = HdrList_new();
    KVPairRef hdrln1 = KVPair_new("KVPairKey1", strlen("KVPairKey1"), "333", strlen("333"));
    KVPairRef hdrln2 = KVPair_new("KVPairKey2", strlen("KVPairKey2"), "4444", strlen("4444"));
    KVPairRef hdrln3 = KVPair_new("KVPairKey3", strlen("KVPairKey2"), "55555", strlen("55555"));
    KVPairRef hdrln4 = KVPair_new("KVPairKey4", strlen("KVPairKey2"), "666666", strlen("666666"));
    KVPairRef x = HdrList_find(hdrlistref, "onetwothree");
    int sz = HdrList_size(hdrlistref);
    UT_EQUAL_INT(sz, 0);
    UT_EQUAL_PTR(x, NULL);


    HdrList_add_back(hdrlistref, hdrln1);
    HdrList_add_back(hdrlistref, hdrln2);
    HdrList_add_back(hdrlistref, hdrln3);
    HdrList_add_back(hdrlistref, hdrln4);
    int sz2 = HdrList_size(hdrlistref);
    UT_EQUAL_INT(sz2, 4);
    CbufferRef cbref = HdrList_serialize(hdrlistref);

    KVPairRef y = HdrList_find(hdrlistref, "onetwothree");
    UT_EQUAL_PTR(y, NULL);
    KVPairRef z = HdrList_find(hdrlistref, "KVPAIRkey1");
    UT_NOT_EQUAL_PTR(z, NULL);
    UT_EQUAL_PTR(((void*)hdrln1),((void*) z) );
    KVPairRef w = HdrList_find(hdrlistref, "KVPAIRKEY2");
    UT_NOT_EQUAL_PTR(w, NULL);
    UT_EQUAL_PTR(((void*)hdrln2),((void*) w) );

    UT_EQUAL_INT(HdrList_size(hdrlistref), 4);
    HdrList_remove(hdrlistref, "onetwothree");
    UT_EQUAL_INT(HdrList_size(hdrlistref), 4);

    // delete one in the middle of the chain
    HdrList_remove(hdrlistref, "KVPairKey3");
    int xx = HdrList_size(hdrlistref);
    UT_EQUAL_INT(HdrList_size(hdrlistref), 3);
    // front of chain
    HdrList_remove(hdrlistref, "KVPairKey1");
    UT_EQUAL_INT(HdrList_size(hdrlistref), 2);

    // back of chain
    HdrList_remove(hdrlistref, "KVPAIRKEY4");
    UT_EQUAL_INT(HdrList_size(hdrlistref), 1);

    // last one
    HdrList_remove(hdrlistref, "KVPAIRKEY2");
    UT_EQUAL_INT(HdrList_size(hdrlistref), 0);

    HdrList_dispose(&hdrlistref);
    Cbuffer_dispose(&cbref);

    return 0;
}
void trial_HdrList_add_line(HdrListRef this, char* label, int lablen, char* value, int vallen)
{
    KVPairRef hl_content_type = KVPair_new(label, lablen, value, vallen);
    HdrList_add_front(this, hl_content_type);
}
int test_serialize_headers()
{
    int body_len = 37;
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    HdrListRef hdrs = HdrList_new();
    KVPairRef hl_content_length = KVPair_new(HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    HdrList_add_front(hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    KVPairRef hl_content_type = KVPair_new(HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));
    HdrList_add_front(hdrs, hl_content_type);
    CbufferRef ser = HdrList_serialize(hdrs);
    free(body_len_str);
    HdrList_dispose(&hdrs);
    Cbuffer_dispose(&ser);
    return 0;
}
int test_serialize_headers_2()
{
    int body_len = 37;
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    HdrListRef hdrs = HdrList_new();
    trial_HdrList_add_line(hdrs, HEADER_CONTENT_LENGTH, strlen(HEADER_CONTENT_LENGTH), body_len_str, strlen(body_len_str));
    char* content_type = "text/html; charset=UTF-8";
    HdrList_add_line(hdrs, HEADER_CONTENT_TYPE, strlen(HEADER_CONTENT_TYPE), content_type, strlen(content_type));

    CbufferRef ser = HdrList_serialize(hdrs);
    free(body_len_str);
    Cbuffer_dispose(&ser);
    HdrList_dispose(&hdrs);
    return 0;
}
int test_hdr_add_many()
{
    HdrListRef hdrs = HdrList_new();
    HdrList_add_cstr(hdrs, "Key1", "value1");
    HdrList_add_cstr(hdrs, "Key2", "value2");
    HdrList_add_cstr(hdrs, "Key3", "value3");
    HdrList_add_cstr(hdrs, "Key4", "value4");
    CbufferRef cb = HdrList_serialize(hdrs);
    UT_EQUAL_CSTR(Cbuffer_cstr(cb), "KEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n");
    printf("This is it\n");
    return 0;
}
int test_hdrlist_ar()
{
    char* ar[][2] = {
        {"Key1", "value1"},
        {"Key2", "value2"},
        {"Key3", "value3"},
        {"Key4", "value4"},
        {NULL, NULL}
    };
    HdrListRef hdrs = HdrList_from_array(ar);

    CbufferRef cb = HdrList_serialize(hdrs);
    UT_EQUAL_CSTR(Cbuffer_cstr(cb), "KEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n");
    return 0;
}
#endif
HttpMessageRef make_request_message()
{
    const char* ar[][2] = {
            {"Key1", "value1"},
            {"Key2", "value2"},
            {"Key3", "value3"},
            {"Key4", "value4"},
            {NULL, NULL}
    };
    HttpMessageRef msg = HttpMessage_new_request();
    HttpMessage_set_method(msg, HTTP_POST);
    HttpMessage_set_target(msg, "/somewhere.php?a=111&b=222");
    HttpMessage_set_headers_arr(msg, ar);
    return msg;
}
HttpMessageRef make_response_message()
{
    const char* ar[][2] = {
            {"Key1", "value1"},
            {"Key2", "value2"},
            {"Key3", "value3"},
            {"Key4", "value4"},
            {NULL, NULL}
    };
    HttpMessageRef msg = HttpMessage_new_response();
    HttpMessage_set_status(msg, 203);
    HttpMessage_set_reason(msg, "AREASON");
    HttpMessage_set_headers_arr(msg, ar);
    return msg;
}
HttpMessageRef make_response_message_empty_body()
{
    const char* ar[][2] = {
            {"Key1", "value1"},
            {"Key2", "value2"},
            {"Key3", "value3"},
            {"Key4", "value4"},
            {NULL, NULL}
    };
    HttpMessageRef msg = HttpMessage_new_response();
    HttpMessage_set_status(msg, 203);
    HttpMessage_set_reason(msg, "AREASON");
    HttpMessage_set_headers_arr(msg, ar);
    HttpMessage_set_body(msg, BufferChain_new());
    return msg;
}
static BufferChainRef make_chain_2()
{
    char* str[5] = {
            (char*)"ABCDEFGH",
            (char*)"IJKLMNOPQ",
            (char*)"RSTUVWXYZ1234",
            NULL
    };
    BufferChainRef bc = BufferChain_new();
    for(int i = 0; i < 3; i++) {
        IOBufferRef iob = IOBuffer_from_cstring(str[i]);
        BufferChain_add_back(bc, iob);
    }
    return bc;
}

HttpMessageRef make_response_message_with_body()
{
    const char* ar[][2] = {
            {"Key1", "value1"},
            {"Key2", "value2"},
            {"Key3", "value3"},
            {"Key4", "value4"},
            {NULL, NULL}
    };
    HttpMessageRef msg = HttpMessage_new_response();
    HttpMessage_set_status(msg, 203);
    HttpMessage_set_reason(msg, "AREASON");
    HttpMessage_set_headers_arr(msg, ar);
    HttpMessage_set_body(msg, make_chain_2());
    return msg;
}

int test_serialize()
{
    {
        HttpMessageRef msg = make_request_message();
        IOBufferRef ser = HttpMessage_serialize(msg);
        const char *correct = "POST /somewhere.php?a=111&b=222 HTTP/1.1\r\nKEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n\r\n";
        const char *candidate = IOBuffer_cstr(ser);
        int r = strcmp(candidate, correct);
        UT_EQUAL_INT(r, 0);
    }
    {
        HttpMessageRef msg = make_response_message();
        IOBufferRef ser = HttpMessage_serialize(msg);
        const char *correct = "HTTP/1.1  203 AREASON\r\nKEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n\r\n";
        const char *candidate = IOBuffer_cstr(ser);
        int r = strcmp(candidate, correct);
        UT_EQUAL_INT(r, 0);
    }
    {
        HttpMessageRef msg = make_response_message_empty_body();
        IOBufferRef ser = HttpMessage_serialize(msg);
        const char *correct = "HTTP/1.1  203 AREASON\r\nKEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n\r\n";
        const char *candidate = IOBuffer_cstr(ser);
        int r = strcmp(candidate, correct);
        UT_EQUAL_INT(r, 0);
    }
    {
        HttpMessageRef msg = make_response_message_with_body();
        IOBufferRef ser = HttpMessage_serialize(msg);
        const char *correct = "HTTP/1.1  203 AREASON\r\nKEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n\r\nABCDEFGHIJKLMNOPQRSTUVWXYZ1234";
        const char *candidate = IOBuffer_cstr(ser);
        int r = strcmp(candidate, correct);
        UT_EQUAL_INT(r, 0);
    }
    return 0;
}
int test_content_length()
{
    HttpMessageRef msg = make_response_message_empty_body();
    HttpMessage_set_content_length(msg, 123);
    IOBufferRef ser = HttpMessage_serialize(msg);
    const char *correct = "HTTP/1.1  203 AREASON\r\nKEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\nCONTENT-LENGTH: 123\r\n\r\n";
    const char *candidate = IOBuffer_cstr(ser);
    int r = strcmp(candidate, correct);
    UT_EQUAL_INT(r, 0);

    return 0;
}
int main()
{
    UT_ADD(test_serialize);
    UT_ADD(test_content_length);
	int rc = UT_RUN();
	return rc;
}