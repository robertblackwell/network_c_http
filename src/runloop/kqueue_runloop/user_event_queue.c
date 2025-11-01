
#include <kqueue_runloop/runloop_internal.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <rbl/logger.h>
#include <common/list.h>

typedef EventQueue* EvfQueuePtr;

static void dealloc(void** p)
{
}
static void mk_fds(EventQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
#ifdef runloop_eventfd_ENABLE
    int fd = -2;//eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    me->readfd = fd;
    me->writefd = fd;
#else
    pipe2(this->pipefds, O_NONBLOCK | O_CLOEXEC);
    this->readfd = this->pipefds[0];
    this->writefd = this->pipefds[1];
#endif
    uint64_t buf;
    while(1) {
        int nread = read(fd, &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);

}
void runloop_event_queue_init(RunloopRef runloop, RunloopEventRef rlevent)
{
    #if 0
    EVENTFD_SET_TAG(rlevent)
    EvfQueuePtr me = rlevent->evqueue;
    me->list = functor_list_new(runloop_MAX_FDS);
    pthread_mutex_init(&(me->queue_mutex), NULL);
    mk_fds(me);
    #endif
}
RunloopEventRef runloop_event_queue_new(RunloopRef rl)
{
    RunloopEventRef tmp = event_allocator_alloc(rl->event_table);
    runloop_event_queue_init(rl, tmp);
    return tmp;
}
void runloop_eventfd_queue_free(RunloopEventRef athis)
{
    event_allocator_free(athis->runloop->event_table, athis);
}
int runloop_eventfd_queue_readfd(RunloopEventRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    return athis->uevent.read_fd;
}
void runloop_eventfd_queue_add(RunloopEventRef athis, Functor item)
{
    EvfQueuePtr me = athis->interthread_queue.queue;
    pthread_mutex_lock(&(me->queue_mutex));
    functor_list_add(me->list, item);
    uint64_t buf = 1;
    write(me->writefd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(me->queue_mutex));
}
Functor runloop_eventfd_queue_remove(RunloopEventRef athis) {
    EvfQueuePtr me = athis->interthread_queue.queue;
    pthread_mutex_lock(&(me->queue_mutex));
    Functor op;
    if (functor_list_size(me->list) > 0) {
        op = functor_list_remove(me->list);
    } else {
        op.f = NULL; op.arg = NULL;
    }
    uint64_t buf;
    int nread = read(me->readfd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(me->queue_mutex));
    // remember to read from the pipe to clear the event
    return op;
}
