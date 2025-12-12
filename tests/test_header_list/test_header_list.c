
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/cbuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <common/alloc.h>
#include <common/alloc_malloc.h>
#include <../../src/common/arena/arena.h>
#include <src/http/http_header_line.h>
#include <src/http/header_list.h>
#include <src/http/http_message.h>


///////////////////////////////////////////////////
int test_header_list_new()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    HeaderListPtr hdrlistref = header_list_new(ma);
    int sz = header_list_size(hdrlistref);
    UT_NOT_EQUAL_PTR(hdrlistref, NULL);
    UT_EQUAL_INT(sz, 0);
    header_list_free(hdrlistref);
    hdrlistref = NULL;
    UT_EQUAL_PTR(hdrlistref, NULL);
    allocator_destroy(ma);
	return 0;
}
int test_header_list_add_back_get_content()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    HeaderListPtr hdrlistref = header_list_new(ma);
    HeaderLinePtr hdrln1 = header_line_from_buffer("KVPairKey1", strlen("KVPairKey1"), "333", strlen("333"), ma);
    HeaderLinePtr hdrln2 = header_line_from_buffer("KVPairKey2", strlen("KVPairKey2"), "4444", strlen("4444"), ma);
    header_list_add_back(hdrlistref, hdrln1);
    header_list_add_back(hdrlistref, hdrln2);
    int sz = header_list_size(hdrlistref);
    UT_EQUAL_INT(sz, 2);
    HeaderLinePtr hdrref1 = header_list_first(hdrlistref);
    const char* sh1 = Cbuffer_cstr(hdrref1->key);
    const char* sv1 = Cbuffer_cstr(hdrref1->value);
    HeaderLinePtr hdrref2 = header_list_last(hdrlistref);
    const char* sh2 = Cbuffer_cstr(hdrref2->key);
    const char* sv2 = Cbuffer_cstr(hdrref2->value);
    UT_EQUAL_INT(strcmp(sh1, "KVPAIRKEY1"), 0);
    UT_EQUAL_INT(strcmp(sv1, "333"), 0);
    UT_EQUAL_INT(strcmp(sh2, "KVPAIRKEY2"), 0);
    UT_EQUAL_INT(strcmp(sv2, "4444"), 0);
    header_list_display(hdrlistref);
    header_list_free(hdrlistref);
    allocator_destroy(ma);
    return 0;
}
int test_header_list_find()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    HeaderListPtr hdrlistref = header_list_new(ma);
    HeaderLinePtr hdrln1 = header_line_from_buffer("KVPairKey1", strlen("KVPairKey1"), "333", strlen("333"), ma);
    HeaderLinePtr hdrln2 = header_line_from_buffer("KVPairKey2", strlen("KVPairKey2"), "4444", strlen("4444"), ma);
    HeaderLinePtr hdrln3 = header_line_from_buffer("KVPairKey3", strlen("KVPairKey2"), "55555", strlen("55555"), ma);
    HeaderLinePtr hdrln4 = header_line_from_buffer("KVPairKey4", strlen("KVPairKey2"), "666666", strlen("666666"), ma);
    HeaderLinePtr x = header_list_find(hdrlistref, "onetwothree");
    int sz = header_list_size(hdrlistref);
    UT_EQUAL_INT(sz, 0);
    UT_EQUAL_PTR(x, NULL);


    header_list_add_back(hdrlistref, hdrln1);
    header_list_add_back(hdrlistref, hdrln2);
    header_list_add_back(hdrlistref, hdrln3);
    header_list_add_back(hdrlistref, hdrln4);
    int sz2 = header_list_size(hdrlistref);
    UT_EQUAL_INT(sz2, 4);
    CbufferRef cbref = header_list_serialize(hdrlistref);

    HeaderLinePtr y = header_list_find(hdrlistref, "onetwothree");
    UT_EQUAL_PTR(y, NULL);
    HeaderLinePtr z = header_list_find(hdrlistref, "KVPAIRkey1");
    UT_NOT_EQUAL_PTR(z, NULL);
    UT_EQUAL_PTR(((void*)hdrln1),((void*) z) );
    HeaderLinePtr w = header_list_find(hdrlistref, "KVPAIRKEY2");
    UT_NOT_EQUAL_PTR(w, NULL);
    UT_EQUAL_PTR(((void*)hdrln2),((void*) w) );

    UT_EQUAL_INT(header_list_size(hdrlistref), 4);
    header_list_remove(hdrlistref, "onetwothree");
    UT_EQUAL_INT(header_list_size(hdrlistref), 4);

    // delete one in the middle of the chain
    header_list_remove(hdrlistref, "KVPairKey3");
    int xx = header_list_size(hdrlistref);
    UT_EQUAL_INT(header_list_size(hdrlistref), 3);
    // front of chain
    header_list_remove(hdrlistref, "KVPairKey1");
    UT_EQUAL_INT(header_list_size(hdrlistref), 2);

    // back of chain
    header_list_remove(hdrlistref, "KVPAIRKEY4");
    UT_EQUAL_INT(header_list_size(hdrlistref), 1);

    // last one
    header_list_remove(hdrlistref, "KVPAIRKEY2");
    UT_EQUAL_INT(header_list_size(hdrlistref), 0);

    header_list_free(hdrlistref);
    Cbuffer_free(cbref);cbref = NULL;

    allocator_destroy(ma);
    return 0;
}
void trial_header_list_add_line(HeaderListPtr this, char* label, int lablen, char* value, int vallen)
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    HeaderLinePtr hl_content_type = header_line_from_buffer(label, lablen, value, vallen, ma);
    header_list_add_front(this, hl_content_type);
}
int test_serialize_headers()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    int body_len = 37;
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    HeaderListPtr hdrs = header_list_new(ma);
    HeaderLinePtr hl_content_length = header_line_from_buffer(
        HEADER_CONTENT_LENGTH,
        strlen(HEADER_CONTENT_LENGTH),
        body_len_str,
        strlen(body_len_str),
        ma);
    header_list_add_front(hdrs, hl_content_length);
    char* content_type = "text/html; charset=UTF-8";
    HeaderLinePtr hl_content_type = header_line_from_buffer(
        HEADER_CONTENT_TYPE,
        strlen(HEADER_CONTENT_TYPE),
        content_type,
        (int)strlen(content_type),
        ma);
    header_list_add_front(hdrs, hl_content_type);
    CbufferRef ser = header_list_serialize(hdrs);
    free(body_len_str);
    header_list_free(hdrs);
    Cbuffer_free(ser);
    allocator_destroy(ma);
    return 0;
}
int test_serialize_headers_2()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    int body_len = 37;
    char* body_len_str;
    asprintf(&body_len_str, "%d", body_len);

    HeaderListPtr hdrs = header_list_new(ma);
    trial_header_list_add_line(hdrs,
        HEADER_CONTENT_LENGTH,
        strlen(HEADER_CONTENT_LENGTH),
        body_len_str,
        (int)strlen(body_len_str));
    char* content_type = "text/html; charset=UTF-8";
    header_list_add_line(hdrs,
        HEADER_CONTENT_TYPE,
        strlen(HEADER_CONTENT_TYPE),
        content_type,
        (int)strlen(content_type));

    CbufferRef ser = header_list_serialize(hdrs);
    free(body_len_str);
    Cbuffer_free(ser);
    header_list_free(hdrs);
    allocator_destroy(ma);
    return 0;
}
int test_hdr_add_many()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    HeaderListPtr hdrs = header_list_new(ma);
    header_list_add_cstr(hdrs, "Key1", "value1");
    header_list_add_cstr(hdrs, "Key2", "value2");
    header_list_add_cstr(hdrs, "Key3", "value3");
    header_list_add_cstr(hdrs, "Key4", "value4");
    CbufferRef cb = header_list_serialize(hdrs);
    UT_EQUAL_CSTR(Cbuffer_cstr(cb), "KEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n");
    printf("This is it\n");
    allocator_destroy(ma);
    return 0;
}
int test_header_list_ar()
{
#ifdef CBUF_ALLOCATOR_MALLOC
    Allocator* ma = malloc_allocator_create();
#else
    Allocator* ma = arena_allocator_create(4*1024);
#endif
    const char* ar[][2] = {
        {"Key1", "value1"},
        {"Key2", "value2"},
        {"Key3", "value3"},
        {"Key4", "value4"},
        {NULL, NULL}
    };
    HeaderListPtr hdrs = header_list_from_array(ar, ma);

    CbufferRef cb = header_list_serialize(hdrs);
    UT_EQUAL_CSTR(Cbuffer_cstr(cb), "KEY1: value1\r\nKEY2: value2\r\nKEY3: value3\r\nKEY4: value4\r\n");
    allocator_destroy(ma);
    return 0;
}
#define HGHGHx
#ifdef HGHGH
int test_list_add_front()
{
    HeaderListPtr lref = header_list_new();
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
    UT_ADD(test_header_list_ar);
	UT_ADD(test_header_list_new);
    UT_ADD(test_header_list_add_back_get_content);
    UT_ADD(test_header_list_find);
    UT_ADD(test_hdr_add_many);
//    UT_ADD(test_list_remove_front);
//    UT_ADD(test_list_remove_back);
//    UT_ADD(test_iter);
    UT_ADD(test_serialize_headers);
    UT_ADD(test_serialize_headers_2);
	int rc = UT_RUN();
	return rc;
}