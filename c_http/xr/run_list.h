#ifndef c_http_xr_run_list_h
#define c_http_xr_run_list_h
#include <c_http/list.h>
#include <c_http/xr/types.h>

typedef ListRef RunListRef;
typedef ListIter RunListIter;

struct Functor_s;
typedef struct Functor_s Functor, *FunctorRef;

FunctorRef Functor_new(XrWatcherRef wr, WatcherCallback f, void* arg);
void Functor_free(FunctorRef this);
void Functor_call(FunctorRef this);

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

#endif