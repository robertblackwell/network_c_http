#include <c_http/runloop/run_list.h>
#include <unistd.h>
#include <stdlib.h>
/**
 * A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
 */
struct Functor_s
{
//    WatcherRef wref; // this is borrowed do not free
    PostableFunction f;
    void *arg;
};

FunctorRef Functor_new(PostableFunction f, void *arg)
{
    FunctorRef this = malloc(sizeof(Functor));
    this->f = f;
    this->arg = arg;
    return this;
}

void Functor_free(FunctorRef this)
{
    free(this);
}

void Functor_call(FunctorRef this)
{
    this->f(this->arg);
}
/**
 * The runlist - is a list of Functor - these are functions that are ready to run.
 */
static void dealloc(void **ptr)
{
    Functor_free((FunctorRef) *ptr);
}

RunListRef RunList_new()
{
    return (RunListRef) List_new(dealloc);
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
void RunList_exec(RunListRef this)
{
    RunListIter iter = RunList_iterator(this);
    while (iter != NULL) {
        FunctorRef fnc = RunList_itr_unpack(this, iter);
        Functor_call(fnc);
        RunListIter next_iter = RunList_itr_next(this, iter);
        RunList_itr_remove(this, &iter);
        iter = next_iter;
    }
}