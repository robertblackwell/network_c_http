#ifndef EPOLL_FUNCTOR_H
#define EPOLL_FUNCTOR_H
#include <runloop/runloop.h>

// typedef struct Functor_s
// {
//     PostableFunction f;
//     void *arg;
// } Functor, *FunctorRef;

/**
 * A Functor is a generic callback - a function pointer (of type PostableFunction) and single anonymous argument.
 *
 * The significant thing is that the function pointer, points to a function that has the correct
 * signature for the RunList
 *
*/
FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_init(FunctorRef funref, PostableFunction f, void* arg);
void Functor_free(FunctorRef athis);
void Functor_call(FunctorRef athis, RunloopRef runloop_ref);
bool Functor_is_empty(FunctorRef f);
void Functor_dealloc(void **p);

typedef struct FunctorList_s {
//    char       tag[RBL_TAG_LENGTH];
    RBL_DECLARE_TAG;
    int        capacity;
    int        head;
    int        tail_plus;
    FunctorRef list; // points to an array of Functor objects
    RBL_DECLARE_END_TAG;
} FunctorList, *FunctorListRef;

/**
 * NOTE: FunctionList acceptS and returnS values of a Functor NOT a pointer
 */
FunctorListRef functor_list_new(int capacity);
void functor_list_free(FunctorListRef flist);
void functor_list_add(FunctorListRef flist, Functor func);
Functor functor_list_remove(FunctorListRef flist);
int functor_list_size(FunctorListRef flist);

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
// struct Functor_s
// {
//     PostableFunction f;
//     void *arg;
// };


#endif