#ifndef c_http_xr_run_list_h
#define c_http_xr_run_list_h
#include <c_http/dsl/list.h>
#include <c_http/aio_api/types.h>

typedef ListRef RunListRef;
typedef ListIter RunListIter;


/**
 * A Functor is a generic callable - a function pointer (of type PostableFunction) and single anonymous argument
 */
struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;

FunctorRef Functor_new(PostableFunction f, void* arg);
void Functor_free(FunctorRef this);
void Functor_call(FunctorRef this);

/**
 * runlist - is a list of Functor - these are functions that are ready to run.
 *
 * Use should be confined to a single thread - no synchronization.
 *
 * Intended to be used within a Reactor or Runloop
 *
 */

RunListRef RunList_new();
void RunList_free();
void RunList_add_back(RunListRef this, FunctorRef f);
FunctorRef RunList_remove_front(RunListRef this);
int RunList_size (RunListRef rl_ref);

FunctorRef RunList_first (RunListRef rl_ref);

FunctorRef RunList_last (RunListRef rl_ref);

FunctorRef RunList_remove_first (RunListRef rl_ref);

FunctorRef RunList_remove_last (RunListRef rl_ref);

FunctorRef RunList_itr_unpack (RunListRef rl_ref, RunListIter iter);

RunListIter RunList_iterator (RunListRef rl_ref);

RunListIter RunList_itr_next (RunListRef rl_ref, RunListIter iter);

void RunList_itr_remove (RunListRef rl_ref, RunListIter *iter);
/**
 * Scan down the run list calling each Functor entry until the list is empty;
 * @param this
 */
void RunList_exec(RunListRef this);

#endif