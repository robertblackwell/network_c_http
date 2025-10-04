
#include "runloop.h"
#include "rl_internal.h"

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <rbl/logger.h>
#include <common/list.h>

typedef EventfdQueue* EvfQueuePtr;

static void dealloc(void** p)
{
}
static void mk_fds(EventfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
#ifdef runloop_eventfd_ENABLE
    int fd = eventfd(0, O_NONBLOCK | O_CLOEXEC);
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
void runloop_eventfd_queue_init(EventfdQueueRef athis)
{
    EVENTFD_SET_TAG(athis)
    EvfQueuePtr me = (EvfQueuePtr)athis;
    me->list = functor_list_new(runloop_MAX_FDS);
    pthread_mutex_init(&(me->queue_mutex), NULL);
    mk_fds(me);
}
EventfdQueueRef runloop_eventfd_queue_new()
{
    EventfdQueueRef tmp = malloc(sizeof(EventfdQueue));
    runloop_eventfd_queue_init(tmp);
    return tmp;
}
void runloop_eventfd_queue_free(EventfdQueueRef athis)
{
    free(athis);
}
int runloop_eventfd_queue_readfd(EventfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    return me->readfd;
}
void runloop_eventfd_queue_add(EventfdQueueRef athis, Functor item)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    pthread_mutex_lock(&(me->queue_mutex));
    functor_list_add(me->list, item);
    uint64_t buf = 1;
    write(me->writefd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(me->queue_mutex));

}
Functor runloop_eventfd_queue_remove(EventfdQueueRef athis) {
    EvfQueuePtr me = (EvfQueuePtr) athis;
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
