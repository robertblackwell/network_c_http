#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
///**
// * A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
// */

//
//
//FunctorRef Functor_new(PostableFunction f, void *arg)
//{
//    FunctorRef this = malloc(sizeof(Functor));
//    this->f = f;
//    this->arg = arg;
//    return this;
//}
//void Functor_init(FunctorRef this, PostableFunction f, void *arg)
//{
//    this->f = f;
//    this->arg = arg;
//}
//
//void Functor_free(FunctorRef this)
//{
//    free(this);
//}
//
//void Functor_call(FunctorRef this, ReactorRef rtor_ref)
//{
//    this->f(rtor_ref, this->arg);
//}
///**
// * The runlist - is a list of Functor - these are functions that are ready to run.
// */
//static void dealloc(void **ptr)
//{
//    Functor_free((FunctorRef) *ptr);
//}

RunListRef RunList_new()
{
    return (RunListRef) List_new(Functor_dealloc);
}

void RunList_dispose(RunListRef *rl_ref_ptr)
{
    List_dispose(rl_ref_ptr);
}

int RunList_size(RunListRef rl_ref)
{
    return List_size(rl_ref);
}

FunctorRef RunList_first(RunListRef rl_ref)
{
    return (FunctorRef) List_first(rl_ref);
}

FunctorRef RunList_last(RunListRef rl_ref)
{
    return (FunctorRef) List_last(rl_ref);
}

FunctorRef RunList_remove_first(RunListRef rl_ref)
{
    return (FunctorRef) List_remove_first(rl_ref);
}

FunctorRef RunList_remove_last(RunListRef rl_ref)
{
    return (FunctorRef) List_remove_last(rl_ref);
}

FunctorRef RunList_itr_unpack(RunListRef rl_ref, RunListIter iter)
{
    return (FunctorRef) List_itr_unpack(rl_ref, iter);
}

RunListIter RunList_iterator(RunListRef rl_ref)
{
    return List_iterator(rl_ref);
}

RunListIter RunList_itr_next(RunListRef rl_ref, RunListIter iter)
{
    return List_itr_next(rl_ref, iter);
}

void RunList_itr_remove(RunListRef rl_ref, RunListIter *iter)
{
    List_itr_remove(rl_ref, iter);
}

void RunList_add_back(RunListRef rl_ref, FunctorRef item)
{
    List_add_back(rl_ref, (void *) item);
}

void RunList_add_front(RunListRef rl_ref, FunctorRef item)
{
    List_add_front(rl_ref, (void *) item);
}


//typedef struct FunctorList_s {
//    int        capacity;
//    int        head;
//    int        tail_plus;
//    FunctorRef list[100];
//} FunctorList, *FunctorListRef;
//
//FunctorListRef functor_list_new(int capacity)
//{
//    FunctorListRef st = malloc(sizeof(FunctorList));
//    st->capacity = capacity;
//    st->head = 0;
//    st->tail_plus = 0;
//    for(int i = 0; i < capacity; i++) {
//        st->list[i] = malloc(sizeof(Functor));
//    }
//    return st;
//}
//void functor_list_add(FunctorListRef lstref, Functor func)
//{
//    lstref->head = (lstref->head + 1) % lstref->capacity;
//    *(lstref->list[lstref->head]) = func;
//}
//int functor_list_size(FunctorListRef lstref)
//{
//    return (lstref->head + lstref->capacity - lstref->tail_plus) % lstref->capacity;
//}
//Functor functor_list_remove(FunctorListRef lstref)
//{
//    if(functor_list_size(lstref) == 0) {
//        assert(false);
//    }
//    int tmpix = (lstref->tail_plus + 1) % lstref->capacity;
//    lstref->tail_plus = tmpix;
//    return *(lstref->list[tmpix]);
//}
