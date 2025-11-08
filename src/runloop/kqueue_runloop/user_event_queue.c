
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
Functor user_event_queue_remove(UserEventQueueRef athis);
int   user_event_queue_readfd(UserEventQueueRef athis);

static void dealloc(void** p)
{
}
#if 0
static void mk_fds(UserEventQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
#ifdef RUNLOOP_USER_EVENT_TWO_PIPE_TRICK
    pipe2(this->pipefds, O_NONBLOCK | O_CLOEXEC);
    this->readfd = this->pipefds[0];
    this->writefd = this->pipefds[1];
#else
    // if not using the two piupe trick, since this is kqueue
    // we use the EVFILT_USER where athis is the unique identifier

#endif
    uint64_t buf;
    while(1) {
        int nread = read(fd, &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);

}
#endif

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
    // user_event_queue_register(ue_queue, queue_triggered_cb, arg);
    runloop_user_event_arm(ue_queue->user_event, queue_triggered_cb, ue_queue);
#endif
}

void user_event_queue_init(RunloopRef rl, UserEventQueueRef uequeue)
{
    RBL_SET_TAG(UEQueue_TAG, uequeue);
    RBL_SET_END_TAG(UEQueue_TAG, uequeue);
    EvfQueuePtr me = (EvfQueuePtr)uequeue;
    me->runloop = rl;
    me->user_event = runloop_user_event_new(rl);
    me->list = functor_list_new(runloop_MAX_FDS);
    pthread_mutex_init(&(me->queue_mutex), NULL);
}
UserEventQueueRef user_event_queue_new(RunloopRef rl)
{
    UserEventQueueRef tmp = malloc(sizeof(UserEventQueue));
    user_event_queue_init(rl, tmp);
    return tmp;
}
void user_event_queue_free(UserEventQueueRef athis)
{
    RBL_CHECK_TAG(UEQueue_TAG, athis);
    RBL_CHECK_END_TAG(UEQueue_TAG, athis);
    runloop_user_event_free(athis->user_event);
    functor_list_free(athis->list);
    free(athis);
}
void user_event_queue_arm(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_register(uequeue->user_event);
    runloop_user_event_arm(uequeue->user_event, queue_triggered_cb, uequeue);
}

void user_event_queue_register(UserEventQueueRef uequeue, UserEventQueueCallback cb, void* cb_arg)
{
    assert(0);
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_register(uequeue->user_event);
    runloop_user_event_arm(uequeue->user_event, queue_triggered_cb, uequeue);
}
void user_event_queue_deregister(UserEventQueueRef uequeue)
{
    RBL_CHECK_TAG(UEQueue_TAG, uequeue);
    RBL_CHECK_END_TAG(UEQueue_TAG, uequeue);
    runloop_user_event_deregister(uequeue->user_event);
}

int user_event_queue_readfd(UserEventQueueRef athis)
{
    RBL_CHECK_TAG(UEQueue_TAG, athis);
    RBL_CHECK_END_TAG(UEQueue_TAG, athis);
    assert(0); // kqueue user_event does not have a readfd
    EvfQueuePtr me = (EvfQueuePtr)athis;
    return -1;
}
void user_event_queue_add(UserEventQueueRef athis, Functor item)
{
    RBL_CHECK_TAG(UEQueue_TAG, athis);
    RBL_CHECK_END_TAG(UEQueue_TAG, athis);
    EvfQueuePtr me = athis;
    pthread_mutex_lock(&(me->queue_mutex));
    if ((me->list != NULL) ) {
        functor_list_add(me->list, item);
        RunloopUserEventRef uevent = athis->user_event;
        kqh_user_event_trigger(uevent, NULL);
    }
    pthread_mutex_unlock(&(me->queue_mutex));
}
Functor user_event_queue_remove(UserEventQueueRef athis) {
    RBL_CHECK_TAG(UEQueue_TAG, athis);
    RBL_CHECK_END_TAG(UEQueue_TAG, athis);
    EvfQueuePtr me = athis;
    pthread_mutex_lock(&(me->queue_mutex));
    Functor op;
    if (functor_list_size(me->list) > 0) {
        op = functor_list_remove(me->list);
    } else {
        op.f = NULL; op.arg = NULL;
    }
    pthread_mutex_unlock(&(me->queue_mutex));
    // remember to read from the pipe to clear the event
    return op;
}
RunloopRef user_event_queue_get_runloop(UserEventQueueRef athis)
{
    RBL_CHECK_TAG(UEQueue_TAG, athis);
    RBL_CHECK_END_TAG(UEQueue_TAG, athis);
    assert(athis->runloop == runloop_user_event_get_runloop(athis->user_event));
    return athis->runloop;
}
void user_event_queue_verify(UserEventQueueRef ueq)
{
    RBL_CHECK_TAG(UEQueue_TAG, ueq);
    RBL_CHECK_END_TAG(UEQueue_TAG, ueq);
}
