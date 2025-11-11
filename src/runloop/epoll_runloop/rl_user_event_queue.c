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
Functor user_event_queue_remove(UserEventQueueRef athis);




void queue_triggered_cb(RunloopRef rl, void* arg)
{
    UserEventQueueRef ue_queue = (UserEventQueueRef)arg;
    RBL_CHECK_TAG(UEQueue_TAG, ue_queue);
    RBL_CHECK_END_TAG(UEQueue_TAG, ue_queue);
    user_event_queue_verify(ue_queue);
    runloop_user_event_verify(ue_queue->user_event);
//    while (1) {
        Functor queue_data = user_event_queue_remove(ue_queue);
        if (queue_data.f == NULL) {
            printf("queue_cb - queue is empty \n");
            return;
        }
        PostableFunction pf = queue_data.f;
        void* postable_arg = queue_data.arg;
        runloop_post(rl, pf, postable_arg);
//    }
#ifdef APPLE_FLAG
    runloop_user_event_arm(ue_queue->user_event, queue_triggered_cb, ue_queue);
#elif defined(LINUX_FLAG)
#endif
}
void runloop_user_event_queue_init(RunloopRef rl, UserEventQueueRef uequeue, size_t capacity)
{
    RBL_SET_TAG(UEQueue_TAG, uequeue)
    RBL_SET_END_TAG(UEQueue_TAG, uequeue)
    EvfQueuePtr me = (EvfQueuePtr)uequeue;
    uequeue->user_event = runloop_user_event_new(rl);
    uequeue->list = functor_list_new(capacity);
    pthread_mutex_init(&(uequeue->queue_mutex), NULL);
}

UserEventQueueRef user_event_queue_new(RunloopRef rl, size_t capacity)
{
    UserEventQueueRef tmp = malloc(sizeof(UserEventQueue));
    runloop_user_event_queue_init(rl, tmp, capacity);
    return tmp;
}
void user_event_queue_free(UserEventQueueRef ueq)
{
    RBL_CHECK_TAG(UEQueue_TAG, ueq)
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq)
    RunloopRef rl = ueq->runloop;
    functor_list_free(ueq->list);
    runloop_user_event_deregister(ueq->user_event);
    runloop_user_event_free(ueq->user_event);
    free(ueq);
}
void user_event_queue_arm(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue)
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue)
    runloop_user_event_arm(uequeue->user_event, queue_triggered_cb, uequeue);
}
void user_event_queue_add(UserEventQueueRef ueq, Functor item)
{
    RBL_CHECK_TAG(UEQueue_TAG, ueq)
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq)
    EvfQueuePtr me = (EvfQueuePtr)ueq;

    pthread_mutex_lock(&(me->queue_mutex));
    functor_list_add(me->list, item);
    runloop_user_event_fire(me->user_event);
    pthread_mutex_unlock(&(me->queue_mutex));

}
Functor user_event_queue_remove(UserEventQueueRef ueq) {
    RBL_CHECK_TAG(UEQueue_TAG, ueq)
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq)
    EvfQueuePtr me = (EvfQueuePtr) ueq;
    pthread_mutex_lock(&(ueq->queue_mutex));
    Functor op;
    if (functor_list_size(ueq->list) > 0) {
        op = functor_list_remove(ueq->list);
    } else {
        op.f = NULL; op.arg = NULL;
    }
    pthread_mutex_unlock(&(ueq->queue_mutex));
    return op;
}
void user_event_queue_verify(UserEventQueueRef ueq)
{
    RBL_CHECK_TAG(UEQueue_TAG, ueq);
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq);
}
