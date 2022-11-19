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
    FunctorListRef st = malloc(sizeof(FunctorList));
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    for(int i = 0; i < capacity; i++) {
        st->list[i] = malloc(sizeof(Functor));
    }
    return st;
}
void functor_list_add(FunctorListRef lstref, Functor func)
{

    int tmp = (lstref->head + 1) % lstref->capacity;
    if(tmp == lstref->tail_plus) {
        CHTTP_ASSERT(false, "functor list is full cannot add another element");
    }
    lstref->head = tmp;
    *(lstref->list[lstref->head]) = func;
}
int functor_list_size(FunctorListRef lstref)
{
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}
Functor functor_list_remove(FunctorListRef lstref)
{
    if(functor_list_size(lstref) == 0) {
        CHTTP_ASSERT(false, "cannot remove an element from an empty list");
    }
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;
    return *(lstref->list[tmpix]);
}
