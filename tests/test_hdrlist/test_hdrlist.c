#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <c_eg/unittest.h>
#include <c_eg/buffer/contig_buffer.h>
#include <c_eg/logger.h>
#include <c_eg/list.h>
#include <c_eg/header_line.h>
#include <c_eg/headerline_list.h>



///////////////////////////////////////////////////
int test_hdrlist_new()
{
    HDRListRef hdrlistref = HDRList_new();
    int sz = HDRList_size(hdrlistref);
    UT_NOT_EQUAL_PTR(hdrlistref, NULL);
    UT_EQUAL_INT(sz, 0);
    HDRList_free(&hdrlistref);
    UT_EQUAL_PTR(hdrlistref, NULL);
	return 0;
}
int test_hdrlist_add_back_get_content()
{
    HDRListRef hdr_listref = HDRList_new();
    HeaderLineRef hdrln1 = HeaderLine_new("HeaderLineKey1", strlen("HeaderLineKey1"), "333", strlen("333"));
    HeaderLineRef hdrln2 = HeaderLine_new("HeaderLineKey2", strlen("HeaderLineKey2"), "4444", strlen("4444"));
    HDRList_add_back(hdr_listref, hdrln1);
    HDRList_add_back(hdr_listref, hdrln2);
    int sz = HDRList_size(hdr_listref);
    UT_EQUAL_INT(sz, 2);
    HeaderLineRef hdrref1 = HDRList_first(hdr_listref);
    char* sh1 = HeaderLine_label(hdrref1);
    char* sv1 = HeaderLine_value(hdrref1);
    HeaderLineRef hdrref2 = HDRList_last(hdr_listref);
    char* sh2 = HeaderLine_label(hdrref2);
    char* sv2 = HeaderLine_value(hdrref2);
    UT_EQUAL_INT(strcmp(sh1, "HEADERLINEKEY1"), 0);
    UT_EQUAL_INT(strcmp(sv1, "333"), 0);
    UT_EQUAL_INT(strcmp(sh2, "HEADERLINEKEY2"), 0);
    UT_EQUAL_INT(strcmp(sv2, "4444"), 0);

    return 0;
}
int test_hdrlist_find()
{
    HDRListRef hdr_listref = HDRList_new();
    HeaderLineRef hdrln1 = HeaderLine_new("HeaderLineKey1", strlen("HeaderLineKey1"), "333", strlen("333"));
    HeaderLineRef hdrln2 = HeaderLine_new("HeaderLineKey2", strlen("HeaderLineKey2"), "4444", strlen("4444"));
    HeaderLineRef hdrln3 = HeaderLine_new("HeaderLineKey3", strlen("HeaderLineKey2"), "55555", strlen("55555"));
    HeaderLineRef hdrln4 = HeaderLine_new("HeaderLineKey4", strlen("HeaderLineKey2"), "666666", strlen("666666"));
    HeaderLineRef x = HDRList_find(hdr_listref, "onetwothree");
    int sz = HDRList_size(hdr_listref);
    UT_EQUAL_INT(sz, 0);
    UT_EQUAL_PTR(x, NULL);


    HDRList_add_back(hdr_listref, hdrln1);
    HDRList_add_back(hdr_listref, hdrln2);
    HDRList_add_back(hdr_listref, hdrln3);
    HDRList_add_back(hdr_listref, hdrln4);
    int sz2 = HDRList_size(hdr_listref);
    UT_EQUAL_INT(sz2, 4);
    CBufferRef cbref = HDRList_serialize(hdr_listref);

    HeaderLineRef y = HDRList_find(hdr_listref, "onetwothree");
    UT_EQUAL_PTR(y, NULL);
    HeaderLineRef z = HDRList_find(hdr_listref, "HeaderLineKey1");
    UT_NOT_EQUAL_PTR(z, NULL);
    UT_EQUAL_PTR(((void*)hdrln1),((void*) z) );
    HeaderLineRef w = HDRList_find(hdr_listref, "HEADERLINEKEY2");
    UT_NOT_EQUAL_PTR(w, NULL);
    UT_EQUAL_PTR(((void*)hdrln2),((void*) w) );

    UT_EQUAL_INT(HDRList_size(hdr_listref), 4);
    HDRList_remove(hdr_listref, "onetwothree");
    UT_EQUAL_INT(HDRList_size(hdr_listref), 4);

    // delete one in the middle of the chain
    HDRList_remove(hdr_listref, "HeaderLineKey3");
    int xx = HDRList_size(hdr_listref);
    UT_EQUAL_INT(HDRList_size(hdr_listref), 3);
    // front of chain
    HDRList_remove(hdr_listref, "HEADERLINEKEY1");
    UT_EQUAL_INT(HDRList_size(hdr_listref), 2);

    // back of chain
    HDRList_remove(hdr_listref, "HEADERLINEKEY4");
    UT_EQUAL_INT(HDRList_size(hdr_listref), 1);

    // last one
    HDRList_remove(hdr_listref, "HEADERLINEKEY2");
    UT_EQUAL_INT(HDRList_size(hdr_listref), 0);


    return 0;
}
#ifdef HGHGH
int test_list_add_front()
{
    ListRef lref = List_new(dealloc);
    DummyObjRef dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    int sz = List_size(lref);
    int v1 = ((DummyObjRef)List_first(lref))->value;
    int v2 = ((DummyObjRef)List_last(lref))->value;
    UT_EQUAL_INT(sz, 1);
    UT_EQUAL_INT(v1, 333);
    UT_EQUAL_INT(v2, 333);
    DummyObjRef dref2 = DummyObj_new(444);
    List_add_front(lref, (void*) dref2);
    int v11 = ((DummyObjRef)List_first(lref))->value;
    int v12 = ((DummyObjRef)List_last(lref))->value;
    UT_EQUAL_INT((List_size(lref)), 2);
    UT_EQUAL_INT(v11, 444);
    UT_EQUAL_INT(v12, 333);

    return 0;
}
int test_list_remove_front()
{
    ListRef lref = List_new(dealloc);
    DummyObjRef dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    List_remove_first(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObjRef dref1 = DummyObj_new(111);
    DummyObjRef dref2 = DummyObj_new(222);
    DummyObjRef dref3= DummyObj_new(333);
    List_add_front(lref, (void*) dref1);
    List_add_front(lref, (void*) dref2);
    List_add_front(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    int v1 = (int)((DummyObjRef)List_remove_first(lref))->value;
    int v2 = (int)((DummyObjRef)List_remove_first(lref))->value;
    int v3 = (int)((DummyObjRef)List_remove_first(lref))->value;
    UT_EQUAL_INT((List_size(lref)), 0);
    UT_EQUAL_INT(v1, 333);
    UT_EQUAL_INT(v2, 222);
    UT_EQUAL_INT(v3, 111);

    return 0;
}
int test_iter()
{
    ListRef lref = List_new(dealloc);
    DummyObjRef dref = DummyObj_new(333);
    List_add_front(lref, (void*) dref);
    List_remove_first(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObjRef dref1 = DummyObj_new(111);
    DummyObjRef dref2 = DummyObj_new(222);
    DummyObjRef dref3= DummyObj_new(333);
    List_add_front(lref, (void*) dref1);
    List_add_front(lref, (void*) dref2);
    List_add_front(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    ListNodeRef iter = List_iterator(lref);
    for(int i = 3; i != 0;i--) {
        DummyObjRef dref = (DummyObjRef)List_itr_unpack(lref, iter);
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
    DummyObjRef dref = DummyObj_new(333);
    List_add_back(lref, (void*) dref);
    List_remove_last(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    DummyObjRef dref1 = DummyObj_new(111);
    DummyObjRef dref2 = DummyObj_new(222);
    DummyObjRef dref3= DummyObj_new(333);
    List_add_back(lref, (void*) dref1);
    List_add_back(lref, (void*) dref2);
    List_add_back(lref, (void*) dref3);
    UT_EQUAL_INT((List_size(lref)), 3);
    DummyObjRef oref1 = (DummyObjRef)List_remove_last(lref);
    DummyObjRef oref2 = (DummyObjRef)List_remove_last(lref);
    DummyObjRef oref3 = (DummyObjRef)List_remove_last(lref);
    UT_EQUAL_INT((List_size(lref)), 0);
    UT_EQUAL_INT((oref1->value), 333);
    UT_EQUAL_INT((oref2->value), 222);
    UT_EQUAL_INT((oref3->value), 111);

    return 0;
}


int test_list_remove_back_one()
{
    ListRef lref = List_new(dealloc);
    DummyObjRef dref = DummyObj_new(333);
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
//    UT_ADD(test_list_add_front);
//    UT_ADD(test_list_remove_front);
//    UT_ADD(test_list_remove_back);
//    UT_ADD(test_iter);
	int rc = UT_RUN();
	return rc;
}