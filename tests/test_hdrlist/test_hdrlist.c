#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <c_http/unittest.h>
#include <c_http/buffer/cbuffer.h>
#include <c_http/logger.h>
#include <c_http/list.h>
#include <c_http/kvpair.h>
#include <c_http/hdrlist.h>
#include <c_http/message.h>



///////////////////////////////////////////////////
int test_hdrlist_new()
{
    HdrListRef hdrlistref = HdrList_new();
    int sz = HdrList_size(hdrlistref);
    UT_NOT_EQUAL_PTR(hdrlistref, NULL);
    UT_EQUAL_INT(sz, 0);
    HdrList_free(&hdrlistref);
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
    HdrList_free(&hdrlistref);
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

    HdrList_free(&hdrlistref);
    Cbuffer_free(&cbref);

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
    HdrList_free(&hdrs);
    Cbuffer_free(&ser);
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
    Cbuffer_free(&ser);
    HdrList_free(&hdrs);
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
int main()
{
	UT_ADD(test_hdrlist_new);
    UT_ADD(test_hdrlist_add_back_get_content);
    UT_ADD(test_hdrlist_find);
    UT_ADD(test_hdr_add_many);
//    UT_ADD(test_list_remove_front);
//    UT_ADD(test_list_remove_back);
//    UT_ADD(test_iter);
    UT_ADD(test_serialize_headers);
    UT_ADD(test_serialize_headers_2);
	int rc = UT_RUN();
	return rc;
}