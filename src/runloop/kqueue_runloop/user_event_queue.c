#include "runloop_internal.h"
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <rbl/logger.h>
#include <common/list.h>

typedef UserEventQueue* EvfQueuePtr;
void user_event_queue_register(UserEventQueueRef uequeue, UserEventQueueCallback cb, void* cb_arg);
void user_event_queue_deregister(UserEventQueueRef uequeue);
Functor user_event_queue_remove(UserEventQueueRef uequeue);
int   user_event_queue_readfd(UserEventQueueRef uequeue);

void queue_triggered_cb(RunloopRef rl, void* arg)
{
    UserEventQueueRef ue_queue = (UserEventQueueRef)arg;
    RBL_CHECK_TAG(UEQueue_TAG, ue_queue);
    RBL_CHECK_END_TAG(UEQueue_TAG, ue_queue);
    user_event_queue_verify(ue_queue);
    runloop_user_event_verify(ue_queue->user_event);
    while (1) {
        Functor queue_data = user_event_queue_remove(ue_queue);
        if (queue_data.f == NULL) {
            printf("queue_cb - queue is empty \n");
            return;
        }
        PostableFunction pf = queue_data.f;
        void* postable_arg = queue_data.arg;
        runloop_post(rl, pf, postable_arg);
    }
#ifdef APPLE_FLAG
    runloop_user_event_arm(ue_queue->user_event, queue_triggered_cb, ue_queue);
#elif defined(LINUX_FLAG)
#endif
}
void user_event_queue_init(RunloopRef rl, UserEventQueueRef uequeue, size_t capacity)
{
    RBL_SET_TAG(UEQueue_TAG, uequeue);
    RBL_SET_END_TAG(UEQueue_TAG, uequeue);
    EvfQueuePtr me = (EvfQueuePtr)uequeue;
    me->runloop = rl; //TODO
    me->user_event = runloop_user_event_new(rl);
    me->list = functor_list_new((int)capacity);
    pthread_mutex_init(&(me->queue_mutex), NULL);
}
UserEventQueueRef user_event_queue_new(RunloopRef rl, size_t capacity)
{
    UserEventQueueRef tmp = malloc(sizeof(UserEventQueue));
    user_event_queue_init(rl, tmp, capacity);
    return tmp;
}
void user_event_queue_free(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_free(uequeue->user_event);
    functor_list_free(uequeue->list);
    free(uequeue);
}
void user_event_queue_arm(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_register(uequeue->user_event);
    runloop_user_event_arm(uequeue->user_event, queue_triggered_cb, uequeue);
}
void user_event_queue_add(UserEventQueueRef uequeue, Functor item)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    EvfQueuePtr me = uequeue;
    pthread_mutex_lock(&(me->queue_mutex));
    if ((me->list != NULL) ) {
        functor_list_add(me->list, item);
        RunloopUserEventRef uevent = uequeue->user_event;
        kqh_user_event_trigger(uevent, NULL);
    }
    pthread_mutex_unlock(&(me->queue_mutex));
}
Functor user_event_queue_remove(UserEventQueueRef uequeue) {
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    EvfQueuePtr me = uequeue;
    pthread_mutex_lock(&(me->queue_mutex));
    Functor op;
    if (functor_list_size(me->list) > 0) {
        op = functor_list_remove(me->list);
    } else {
        op.f = NULL; op.arg = NULL;
    }
    pthread_mutex_unlock(&(me->queue_mutex));
    return op;
}
void user_event_queue_verify(UserEventQueueRef ueq)
{
    RBL_CHECK_TAG(UEQueue_TAG, ueq);
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq);
}
// todo check we need all of these
RunloopRef user_event_queue_get_runloop(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    assert(uequeue->runloop == runloop_user_event_get_runloop(uequeue->user_event));
    return uequeue->runloop;
}
//todo
void user_event_queue_register(UserEventQueueRef uequeue, UserEventQueueCallback cb, void* cb_arg)
{
    assert(0);
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_register(uequeue->user_event);
    runloop_user_event_arm(uequeue->user_event, queue_triggered_cb, uequeue);
}
//todo 
void user_event_queue_deregister(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_deregister(uequeue->user_event);
}
// todo
int user_event_queue_readfd(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    assert(0); // kqueue user_event does not have a readfd
    EvfQueuePtr me = (EvfQueuePtr)uequeue;
    return -1;
}
