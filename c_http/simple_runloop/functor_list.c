#define _GNU_SOURCE
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <c_http/macros.h>

//typedef struct FunctorList_s {
//    int        capacity;
//    int        head;
//    int        tail_plus;
//    FunctorRef list[100];
//} FunctorList, *FunctorListRef;

FunctorListRef functor_list_new(int capacity)
{
    CHTTP_ASSERT((capacity <= RTOR_READY_LIST_MAX), "Functor List capacity is too big");
    FunctorListRef st = malloc(sizeof(FunctorList));
    XR_FNCLST_SET_TAG(st)
    SET_TAG_FIELD(FunctorList_TAG, st, end_tag)
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    for(int i = 0; i < capacity; i++) {
        st->list[i] = (FunctorRef)malloc(sizeof(Functor)*2);
//        st->list[i] = malloc(sizeof(Functor)*2);
    }
    XR_FNCLST_CHECK_TAG(st);
    CHECK_TAG_FIELD(FunctorList_TAG, st, end_tag)
    return st;
}
void functor_list_free(FunctorListRef this)
{
//    return;
    for(int i = 0; i < this->capacity; i++) {
        free(this->list[i]);
    }
    free(this);
}
void functor_list_add(FunctorListRef lstref, Functor func)
{
    XR_FNCLST_CHECK_TAG(lstref);
    CHECK_TAG_FIELD(FunctorList_TAG, lstref, end_tag)

    int tmp = (lstref->head + 1) % lstref->capacity;
    if(tmp == lstref->tail_plus) {
        CHTTP_ASSERT(false, "functor list is full cannot add another element");
    }
    lstref->head = tmp;
    *(lstref->list[lstref->head]) = func;
    CHECK_TAG_FIELD(FunctorList_TAG, lstref, end_tag)
}
int functor_list_size(FunctorListRef lstref)
{
    XR_FNCLST_CHECK_TAG(lstref);
    CHECK_TAG_FIELD(FunctorList_TAG, lstref, end_tag)
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}
Functor functor_list_remove(FunctorListRef lstref)
{
    XR_FNCLST_CHECK_TAG(lstref);
    CHECK_TAG_FIELD(FunctorList_TAG, lstref, end_tag)
    if(functor_list_size(lstref) == 0) {
        CHTTP_ASSERT(false, "cannot remove an element from an empty list");
    }
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;
    CHECK_TAG_FIELD(FunctorList_TAG, lstref, end_tag)
    return *(lstref->list[tmpix]);
}
