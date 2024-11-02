
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <rbl/macros.h>

// TODO - make these macros at some point
static FunctorRef addr_of_functor_entry(FunctorListRef lstref, int index)
{
    return (&(lstref->list[index]));
}
static void set_functor_entry(FunctorListRef lstref, int index, Functor func)
{
    lstref->list[index] = func;
}
static Functor get_functor_entry(FunctorListRef lstref, int index)
{
    return (lstref->list[index]);
}
FunctorListRef functor_list_new(int capacity)
{
    RBL_ASSERT((capacity <= runloop_READY_LIST_MAX), "Functor List capacity is too big");
    FunctorListRef st = malloc(sizeof(FunctorList));
    FNCLST_SET_TAG(st)
    RBL_SET_END_TAG(FunctorList_TAG, st)
    st->capacity = capacity;
    st->head = 0;
    st->tail_plus = 0;
    st->list = malloc(sizeof(Functor) * (capacity + 1));
    FNCLST_CHECK_TAG(st);
    RBL_CHECK_END_TAG(FunctorList_TAG, st)
    return st;
}
void functor_list_free(FunctorListRef this)
{
    free(this->list);
    free(this);
}
void functor_list_add(FunctorListRef lstref, Functor func)
{
    FNCLST_CHECK_TAG(lstref);
    RBL_CHECK_END_TAG(FunctorList_TAG, lstref)
    int tmp = (lstref->head + 1) % lstref->capacity;
    if(tmp == lstref->tail_plus) {
        RBL_ASSERT(false, "functor list is full cannot add another element");
    }
    lstref->head = tmp;
    set_functor_entry(lstref, (lstref->head), func);
    RBL_CHECK_END_TAG(FunctorList_TAG, lstref)
}
int functor_list_size(FunctorListRef lstref)
{
    FNCLST_CHECK_TAG(lstref);
    RBL_CHECK_END_TAG(FunctorList_TAG, lstref)
    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
}
Functor functor_list_remove(FunctorListRef lstref)
{
    FNCLST_CHECK_TAG(lstref);
    RBL_CHECK_END_TAG(FunctorList_TAG, lstref)
    if(functor_list_size(lstref) == 0) {
        RBL_ASSERT(false, "cannot remove an element from an empty list");
    }
    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
    lstref->tail_plus = tmpix;
    RBL_CHECK_END_TAG(FunctorList_TAG, lstref)

    return get_functor_entry(lstref, tmpix);
}
