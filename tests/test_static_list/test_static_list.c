
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/cbuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <src//runloop/runloop.h>
#include <src/runloop/rl_internal.h>

//typedef struct DummyObj_s {
//    long value;
//    char str_value[8];
//} DummyObj;
//
//
//DummyObj* DummyObj_new(long val, char* str_value)
//{
//    DummyObj* dref = malloc(sizeof(DummyObj));
//    dref->value = val;
//    if(strlen(str_value) < 8) {
//        strncpy(dref->str_value, str_value, strlen(str_value));
//    } else {
//        strncpy(dref->str_value, str_value, 8);
//    }
//}
//void DummyObj_init(DummyObj* dref, long val, char* str_value)
//{
//    dref->value = val;
//    if(strlen(str_value) < 8) {
//        strncpy(dref->str_value, str_value, strlen(str_value));
//    } else {
//        strncpy(dref->str_value, str_value, 8);
//    }
//}
//void DummyObj_free(DummyObj** dref)
//{
//
//    free((void*)*dref);
//    *dref = NULL;
//}

///////////////////////////////////////////////////
int test_List_new()
{
    FunctorRef tab = malloc(10*sizeof(Functor));
    for(int i = 0; i < 10; i++) {
        const char* pch = (const char*)&(tab[i]);
        memset((void*)pch, (int)('A') + i, sizeof(Functor));
    }
    FunctorListRef lref = functor_list_new(10);
    int sz = functor_list_size(lref);
    UT_EQUAL_INT(sz, 0);
	return 0;
}
int test_list_add()
{
    FunctorListRef lref = functor_list_new(10);
    Functor fobj;
    Functor_init(&fobj, NULL, "123456");
    functor_list_add(lref, fobj);
    int sz = functor_list_size(lref);
    UT_EQUAL_INT(sz, 1);
    Functor obj2;
    Functor_init(&obj2, (PostableFunction)444, "44444");
    functor_list_add(lref, obj2);
    UT_EQUAL_INT((functor_list_size(lref)), 2);
    Functor obj3 = functor_list_remove(lref);
    int sz2 = functor_list_size(lref);
    UT_EQUAL_INT((functor_list_size(lref)), 1);
    Functor obj4 = functor_list_remove(lref);
    int sz3 = functor_list_size(lref);
    UT_EQUAL_INT((functor_list_size(lref)), 0);
    return 0;
}
//int test_list_add_front()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_front(lref, (void*) dref);
//    int sz = List_size(lref);
//    int v1 = ((DummyObj*)List_first(lref))->value;
//    int v2 = ((DummyObj*)List_last(lref))->value;
//    UT_EQUAL_INT(sz, 1);
//    UT_EQUAL_INT(v1, 333);
//    UT_EQUAL_INT(v2, 333);
//    DummyObj* dref2 = DummyObj_new(444);
//    List_add_front(lref, (void*) dref2);
//    int v11 = ((DummyObj*)List_first(lref))->value;
//    int v12 = ((DummyObj*)List_last(lref))->value;
//    UT_EQUAL_INT((List_size(lref)), 2);
//    UT_EQUAL_INT(v11, 444);
//    UT_EQUAL_INT(v12, 333);
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}
//int test_list_remove_front()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_front(lref, (void*) dref);
//    List_remove_first(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    DummyObj* dref1 = DummyObj_new(111);
//    DummyObj* dref2 = DummyObj_new(222);
//    DummyObj* dref3= DummyObj_new(333);
//    List_display(lref);
//    List_add_front(lref, (void*) dref1);
//    List_display(lref);
//    List_add_front(lref, (void*) dref2);
//    List_display(lref);
//    List_add_front(lref, (void*) dref3);
//    List_display(lref);
//    UT_EQUAL_INT((List_size(lref)), 3);
//    int v1 = (int)((DummyObj*)List_remove_first(lref))->value;
//    int v2 = (int)((DummyObj*)List_remove_first(lref))->value;
//    int v3 = (int)((DummyObj*)List_remove_first(lref))->value;
//    UT_EQUAL_INT((List_size(lref)), 0);
//    UT_EQUAL_INT(v1, 333);
//    UT_EQUAL_INT(v2, 222);
//    UT_EQUAL_INT(v3, 111);
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}
//
//int test_list_remove_back()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_front(lref, (void*) dref);
//    List_remove_first(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    DummyObj* dref1 = DummyObj_new(111);
//    DummyObj* dref2 = DummyObj_new(222);
//    DummyObj* dref3= DummyObj_new(333);
//    List_display(lref);
//    List_add_back(lref, (void*) dref1);
//    List_display(lref);
//    List_add_back(lref, (void*) dref2);
//    List_display(lref);
//    List_add_back(lref, (void*) dref3);
//    List_display(lref);
//    UT_EQUAL_INT((List_size(lref)), 3);
//    int v1 = (int)((DummyObj*)List_remove_last(lref))->value;
//    int v2 = (int)((DummyObj*)List_remove_last(lref))->value;
//    int v3 = (int)((DummyObj*)List_remove_last(lref))->value;
//    UT_EQUAL_INT((List_size(lref)), 0);
//    UT_EQUAL_INT(v1, 333);
//    UT_EQUAL_INT(v2, 222);
//    UT_EQUAL_INT(v3, 111);
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}
//
//
//int test_iter()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_front(lref, (void*) dref);
//    List_remove_first(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    DummyObj* dref1 = DummyObj_new(111);
//    DummyObj* dref2 = DummyObj_new(222);
//    DummyObj* dref3= DummyObj_new(333);
//    List_add_front(lref, (void*) dref1);
//    List_add_front(lref, (void*) dref2);
//    List_add_front(lref, (void*) dref3);
//    UT_EQUAL_INT((List_size(lref)), 3);
//    ListIterator iter = List_iterator(lref);
//    for(int i = 3; i != 0;i--) {
//        DummyObj* dref = (DummyObj*)List_itr_unpack(lref, iter);
//        int v1 = i*100 + i*10 + i;
//        int v2 = dref->value;
//        UT_EQUAL_INT(v1, v2);
//        iter = List_itr_next(lref, iter);
//    }
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}
//int test_list_remove_backx()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_back(lref, (void*) dref);
//    List_remove_last(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    DummyObj* dref1 = DummyObj_new(111);
//    DummyObj* dref2 = DummyObj_new(222);
//    DummyObj* dref3= DummyObj_new(333);
//    List_add_back(lref, (void*) dref1);
//    List_add_back(lref, (void*) dref2);
//    List_add_back(lref, (void*) dref3);
//    UT_EQUAL_INT((List_size(lref)), 3);
//    DummyObj* oref1 = (DummyObj*)List_remove_last(lref);
//    DummyObj* oref2 = (DummyObj*)List_remove_last(lref);
//    DummyObj* oref3 = (DummyObj*)List_remove_last(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    UT_EQUAL_INT((oref1->value), 333);
//    UT_EQUAL_INT((oref2->value), 222);
//    UT_EQUAL_INT((oref3->value), 111);
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}
//
//
//int test_list_remove_back_one()
//{
//    ListRef lref = List_new(dealloc);
//    DummyObj* dref = DummyObj_new(333);
//    List_add_front(lref, (void*) dref);
//    List_remove_last(lref);
//    UT_EQUAL_INT((List_size(lref)), 0);
//    List_dispose(&lref);
//    UT_EQUAL_PTR(lref, NULL);
//    return 0;
//
//    return 0;
//}

int main()
{
	UT_ADD(test_List_new);
    UT_ADD(test_list_add);
//    UT_ADD(test_list_add_front);
//    UT_ADD(test_list_remove_front);
//    UT_ADD(test_list_remove_back);
//    UT_ADD(test_iter);
	int rc = UT_RUN();
	return rc;
}