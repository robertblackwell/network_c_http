#include "runloop_internal.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
/* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>

#include <sys/epoll.h>
#include <rbl/macros.h>
#include <common/utils.h>
//struct InterthreadQueue_s;
//typedef struct InterthreadQueue_s InterthreadQueue, *InterthreadQueueRef;
//InterthreadQueueRef itqueue_new(RunloopRef rl);
//void itqueue_add(InterthreadQueueRef qref, Functor func);
//Functor itqueue_remove(InterthreadQueueRef qref);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InterthreadQueue
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void itqueue_postable(RunloopRef rl, void* itq_ref_arg)//(RunloopQueueWatcherRef qwref, uint64_t events)
{
    InterthreadQueueRef itq_ref = itq_ref_arg;
//    void* ctx = qwref->queue_postable_arg;
//    RunloopRef rl = runloop_queue_watcher_get_reactor(qwref);
    EventfdQueueRef queue = itq_ref->queue;
    Functor queue_data = runloop_eventfd_queue_remove(queue);
    RBL_ASSERT((!Functor_is_empty(&(queue_data))), "An empty entry in a func list");
    PostableFunction pf = queue_data.f;
    void* postable_arg = queue_data.arg;
    runloop_post(rl, pf, postable_arg);
}

#define InterThreadQ_TYPE "itQu"

InterthreadQueueRef itqueue_new(RunloopRef rl)
{
    InterthreadQueueRef itq_ref = malloc(sizeof(InterthreadQueue));
    RBL_SET_TAG(InterThreadQ_TYPE, itq_ref);
    RBL_SET_END_TAG(InterThreadQ_TYPE, itq_ref)
    itq_ref->runloop = rl;
    itq_ref->queue = runloop_eventfd_queue_new();
    itq_ref->qwatcher_ref = runloop_queue_watcher_new(itq_ref->runloop, itq_ref->queue);
    runloop_queue_watcher_register(itq_ref->qwatcher_ref, &itqueue_postable, itq_ref);
    return itq_ref;
}
void itqueue_add(InterthreadQueueRef qref, Functor func)
{
    runloop_eventfd_queue_add(qref->queue, func);
}
Functor itqueue_remove(InterthreadQueueRef qref)
{
    Functor func = runloop_eventfd_queue_remove(qref->queue);
    return func;
}
