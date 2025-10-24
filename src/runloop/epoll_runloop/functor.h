#ifndef EPOLL_FUNCTOR_H
#define EPOLL_FUNCTOR_H
#include <runloop/runloop.h>
/**
 * A Functor is a generic callback - a function pointer (of type PostableFunction) and single anonymous argument.
 *
 * The significant thing is that the function pointer, points to a function that has the correct
 * signature for the RunList
 *
*/
 struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_init(FunctorRef funref, PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis, RunloopRef runloop_ref);
bool Functor_is_empty(FunctorRef f);
void Functor_dealloc(void **p);
struct Functor_s
{
    PostableFunction f;
    void *arg;
};


#endif