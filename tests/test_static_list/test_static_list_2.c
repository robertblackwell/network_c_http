
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/cbuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>

typedef struct DummyObj_s {
    long value;
    char str_value[8];
} DummyObj;


#define MYTYPE DummyObj
#define MYPREF dummy_obj

#define LIST_STRUCT(T) T##_StaticList_s
#define LIST_TYPE(T)   T##_StaticList
#define LIST_REF(T)    T##_StaticListRef
typedef struct LIST_STRUCT(DummyObj) {
    int       capacity;
    int       head;
    int       tail_plus;
    MYTYPE* list[100];
} LIST_TYPE(DummyObj);
typedef LIST_TYPE(DummyObj) * LIST_REF(DummyObj);



LIST_REF(MYTYPE##) MYPREF_static_list_new(int capacity)
{
    LIST_REF st = malloc(sizeof(LIST_REF));
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    for(int i = 0; i < capacity; i++) {
        st->list[i] = malloc(sizeof(DummyObj));
    }
    return st;
}

void MYPREF_static_list_add(LIST_REF lstref, MYTYPE dobj)
{
    lstref->head = (lstref->head + 1) % lstref->capacity;
    *(lstref->list[lstref->head]) = dobj;
}
MYTYPE MY_PREF_static_list_remove(LIST_REF  lstref)
{
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;
    return *(lstref->list[tmpix]);
}
int MY_PREF_static_list_size(LIST_REF lstref)
{
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}


DummyObj* DummyObj_new(long val, char* str_value)
{
    DummyObj* dref = malloc(sizeof(DummyObj));
    dref->value = val;
    if(strlen(str_value) < 8) {
        strncpy(dref->str_value, str_value, strlen(str_value));
    } else {
        strncpy(dref->str_value, str_value, 8);
    }
}
void DummyObj_init(DummyObj* dref, long val, char* str_value)
{
    dref->value = val;
    if(strlen(str_value) < 8) {
        strncpy(dref->str_value, str_value, strlen(str_value));
    } else {
        strncpy(dref->str_value, str_value, 8);
    }
}
void DummyObj_free(DummyObj** dref)
{

    free((void*)*dref);
    *dref = NULL;
}

///////////////////////////////////////////////////
int test_List_new()
{
    Dummy_StaticListRef lref = static_list_new(10);
    int sz = dummy_obj_static_list_size(lref);
    UT_EQUAL_INT(sz, 0);
	return 0;
}
int test_list_add()
{
    DummyObj_StaticListRef lref = static_list_new(10);
    DummyObj dobj;
    DummyObj_init(&dobj, 333, "123456");
    dummy_obj_static_list_add(lref, dobj);
    int sz = dummy_obj_static_list_size(lref);
    UT_EQUAL_INT(sz, 1);
    DummyObj dobj2;
    DummyObj_init(&dobj2, 444, "44444");
    dummy_obj_static_list_add(lref, dobj2);
    UT_EQUAL_INT((dummy_obj_static_list_size(lref)), 2);
    DummyObj dobj3 = dummy_obj_static_list_remove(lref);
    int sz2 = dummy_obj_static_list_size(lref);
    UT_EQUAL_INT((dummy_obj_static_list_size(lref)), 1);
    DummyObj dobj4 = static_list_remove(lref);
    int sz3 = static_list_size(lref);
    UT_EQUAL_INT((static_list_size(lref)), 0);
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