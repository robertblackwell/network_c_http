#include "runloop.h"
#include "rl_internal.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
///**
// * A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
// */

FunctorRef Functor_new(PostableFunction f, void *arg)
{
    FunctorRef this = malloc(sizeof(Functor));
    this->f = f;
    this->arg = arg;
    return this;
}
void Functor_init(FunctorRef this, PostableFunction f, void *arg)
{
    this->f = f;
    this->arg = arg;
}

void Functor_free(FunctorRef this)
{
    free(this);
}
bool Functor_is_empty(FunctorRef this)
{
    return(this->f == NULL && this->arg == NULL);
}
void Functor_call(FunctorRef this, RunloopRef runloop_ref)
{
    this->f(runloop_ref, this->arg);
}
/**
 * The runlist - is a list of Functor - these are functions that are ready to run.
 */
void Functor_dealloc(void **ptr)
{
    Functor_free((FunctorRef) *ptr);
}

