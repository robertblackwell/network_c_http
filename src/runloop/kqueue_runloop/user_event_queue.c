
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
void runloop_event_queue_init(RunloopRef rl, UserEventQueueRef aq)
{
    USER_EVENT_SET_TAG(aq);
    EvfQueuePtr me = (EvfQueuePtr)aq;
    me->user_event_queue.list = functor_list_new(runloop_MAX_FDS);
    pthread_mutex_init(&(me->user_event_queue.queue_mutex), NULL);
}
UserEventQueueRef runloop_user_event_queue_new(RunloopRef rl)
{
    UserEventQueueRef tmp = event_table_get_entry(rl->event_table);
    runloop_event_queue_init(rl, tmp);
    return tmp;
}
void runloop_user_event_queue_free(UserEventQueueRef athis)
{
    event_table_release_entry(athis->runloop->event_table, athis);
}
int runloop_user_event_queue_readfd(UserEventQueueRef athis)
{
    assert(0); // kqueue user_event does not have a readfd
    EvfQueuePtr me = (EvfQueuePtr)athis;
    return -1;
}
void runloop_user_event_queue_add(UserEventQueueRef athis, Functor item)
{
    EvfQueuePtr me = athis;
    pthread_mutex_lock(&(me->user_event_queue.queue_mutex));
    if ((me->user_event_queue.list != NULL) ) {
        functor_list_add(me->user_event_queue.list, item);
        kqh_user_event_queue_trigger(athis, NULL);
    }
    pthread_mutex_unlock(&(me->user_event_queue.queue_mutex));
}
Functor runloop_user_event_queue_remove(UserEventQueueRef athis) {
    EvfQueuePtr me = athis;
    pthread_mutex_lock(&(me->user_event_queue.queue_mutex));
    Functor op;
    if (functor_list_size(me->user_event_queue.list) > 0) {
        op = functor_list_remove(me->user_event_queue.list);
    } else {
        op.f = NULL; op.arg = NULL;
    }
    pthread_mutex_unlock(&(me->user_event_queue.queue_mutex));
    // remember to read from the pipe to clear the event
    return op;
}
RunloopRef runloop_user_event_queue_get_runloop(UserEventQueueRef athis)
{
    return athis->runloop;
}
