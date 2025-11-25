
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/cbuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <src/http/kvpair.h>
#include <src/http/http_header.h>
#include <src/http/http_message.h>


void free_kvpair(void* p) {
    KVPair_free((KVPairRef) p);
}
///////////////////////////////////////////////////
int test_http_headers_new()
{
    HttpHeaders* headers = http_header_new(100, 4005);
    int sz = http_header_size(headers);
    UT_NOT_EQUAL_PTR(headers, NULL);
    UT_EQUAL_INT(sz, 0);
    http_header_free(headers);
    headers = NULL;
    UT_EQUAL_PTR(headers, NULL);
	return 0;
}
int test_http_headers_set()
{
    HttpHeaders* hdrs = http_header_new(100, 10*1024);
    char* key1 = "KVPairKey1";
    int klen1 = strlen(key1);
    char* value1 = "KVPairValue1";
    int vlen1 = strlen(value1);
    char* expected1 = "KVPairKey1:KVPairValue1\r\n";
    int expected_len1 = strlen(expected1);
    UT_TRUE(!hdrs->waiting_for_value);
    UT_TRUE(hdrs->next_offset == 0);
    UT_TRUE(hdrs->size == 0)
    http_header_set_key(hdrs, key1, klen1);
    UT_TRUE(hdrs->waiting_for_value);
    UT_TRUE((hdrs->next_offset == (klen1+1)));
    UT_TRUE(hdrs->size == 0)
    UT_TRUE(strncmp(hdrs->lines_buffer_start, key1, klen1) == 0)
    http_header_set_value(hdrs, value1, vlen1);
    UT_TRUE(!hdrs->waiting_for_value);
    UT_TRUE((hdrs->next_offset == (expected_len1)));
    UT_TRUE(hdrs->size == 1)
    UT_TRUE(strncmp(hdrs->lines_buffer_start, expected1, expected_len1) == 0)
    size_t sz = http_header_size(hdrs);
    UT_TRUE(sz == 1);

    char* key2 = "KV_Pair_Key_2";
    int klen2 = strlen(key2);
    char* value2 = "KV_Pair_Value_2";
    int vlen2 = strlen(value2);
    char* expected2 = "KVPairKey1:KVPairValue1\r\nKV_Pair_Key_2:KV_Pair_Value_2\r\n";
    int expected_len2 = strlen(expected2);

    http_header_set_key(hdrs, key2, klen2);
    http_header_set_value(hdrs, value2, vlen2);
    UT_TRUE(!hdrs->waiting_for_value);
    UT_TRUE((hdrs->next_offset == (expected_len2)));
    UT_TRUE(hdrs->size == 2)
    UT_TRUE(strncmp(hdrs->lines_buffer_start, expected2, expected_len2) == 0)
    size_t sz2 = http_header_size(hdrs);
    UT_TRUE(sz2 == 2);
    return 0;
}
#if 0
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

    HdrList_safe_free(hdrlistref);
    Cbuffer_free(cbref);cbref = NULL;

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
    HdrList_safe_free(hdrs);
    Cbuffer_free(ser);
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
    Cbuffer_free(ser);
    HdrList_safe_free(hdrs);
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
    const char* ar[][2] = {
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
#ifdef HGHGH
int test_list_add_front()
{
    ListRef lref = List_new(dealloc);
    DummyObj* dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    int sz = List_size(lref);
    int v1 = ((DummyObj*)List_first(lref))->value;
    int v2 = ((DummyObj*)List_last(lref))->value;
    UT_EQUAL_INT(sz, 1);
    UT_EQUAL_INT(v1, 333);
    UT_EQUAL_INT(v2, 333);
    DummyObj* dref2 = DummyObj_new(444);
    List_add_front(lref, (void*) dref2);
    int v11 = ((DummyObj*)List_first(lref))->value;
    int v12 = ((DummyObj*)List_last(lref))->value;
    UT_EQUAL_INT((List_size(lref)), 2);
    UT_EQUAL_INT(v11, 444);
    UT_EQUAL_INT(v12, 333);

    return 0;
}
int test_list_remove_front()
{
    ListRef lref = List_new(dealloc);
    DummyObj* dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    List_remove_first(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObj* dref1 = DummyObj_new(111);
    DummyObj* dref2 = DummyObj_new(222);
    DummyObj* dref3= DummyObj_new(333);
    List_add_front(lref, (void*) dref1);
    List_add_front(lref, (void*) dref2);
    List_add_front(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    int v1 = (int)((DummyObj*)List_remove_first(lref))->value;
    int v2 = (int)((DummyObj*)List_remove_first(lref))->value;
    int v3 = (int)((DummyObj*)List_remove_first(lref))->value;
    UT_EQUAL_INT((List_size(lref)), 0);
    UT_EQUAL_INT(v1, 333);
    UT_EQUAL_INT(v2, 222);
    UT_EQUAL_INT(v3, 111);

    return 0;
}
int test_iter()
{
    ListRef lref = List_new(dealloc);
    DummyObj* dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    List_remove_first(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObj* dref1 = DummyObj_new(111);
    DummyObj* dref2 = DummyObj_new(222);
    DummyObj* dref3= DummyObj_new(333);
    List_add_front(lref, (void*) dref1);
    List_add_front(lref, (void*) dref2);
    List_add_front(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    ListIterator iter = List_iterator(lref);
    for(int i = 3; i != 0;i--) {
        DummyObj* dref = (DummyObj*)List_itr_unpack(lref, iter);
        int v1 = i*100 + i*10 + i;
        int v2 = dref->value;
        UT_EQUAL_INT(v1, v2);
        iter = List_itr_next(lref, iter);
    }
    return 0;
}
int test_list_remove_back()
{
    ListRef lref = List_new(dealloc);
    DummyObj* dref = DummyObj_new(333);
    List_add_back(lref, (void*) dref);
    List_remove_last(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObj* dref1 = DummyObj_new(111);
    DummyObj* dref2 = DummyObj_new(222);
    DummyObj* dref3= DummyObj_new(333);
    List_add_back(lref, (void*) dref1);
    List_add_back(lref, (void*) dref2);
    List_add_back(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    DummyObj* oref1 = (DummyObj*)List_remove_last(lref);
    DummyObj* oref2 = (DummyObj*)List_remove_last(lref);
    DummyObj* oref3 = (DummyObj*)List_remove_last(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    UT_EQUAL_INT((oref1->value), 333);
    UT_EQUAL_INT((oref2->value), 222);
    UT_EQUAL_INT((oref3->value), 111);

    return 0;
}


int test_list_remove_back_one()
{
    ListRef lref = List_new(dealloc);
    DummyObj* dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    List_remove_last(lref);
    UT_EQUAL_INT((List_size(lref)), 0);

    return 0;
}
#endif
#endif
int main()
{
    UT_ADD(test_http_headers_new);
	UT_ADD(test_http_headers_set);
	int rc = UT_RUN();
	return rc;
}