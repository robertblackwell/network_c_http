#define _GNU_SOURCE
#include <c_http/simple_runloop/runloop.h>
#include <c_http//simple_runloop/rl_internal.h>

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <c_http/logger.h>
#include <c_http/common/list.h>

// enables use of eventfd rather than two pipe trick
//#define  C_HTTP_EFD_QUEUE

//typedef struct EvfdQueue_s {
//    ListRef         list;
//    pthread_mutex_t queue_mutex;
//#ifdef C_HTTP_EFD_QUEUE
//#else
//    int             pipefds[2];
//#endif
//    int             readfd;
//    int             writefd;
//    int             id;
//} EvfdQueue;

typedef EvfdQueue* EvfQueuePtr;

static void dealloc(void** p)
{
}
static void mk_fds(EvfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
#ifdef RTOR_EVENTFD_ENABLE
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
void Evfdq_init(EvfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    me->list = functor_list_new(RTOR_MAX_FDS);
    pthread_mutex_init(&(me->queue_mutex), NULL);
    mk_fds(me);
}
EvfdQueueRef Evfdq_new()
{
    EvfdQueueRef tmp = malloc(sizeof(EvfdQueue));
    Evfdq_init(tmp);
    return tmp;
}
void Evfdq_free(EvfdQueueRef this)
{
    free(this);
}
int Evfdq_readfd(EvfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    return me->readfd;
}
void Evfdq_add(EvfdQueueRef athis, Functor item)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    pthread_mutex_lock(&(me->queue_mutex));
    functor_list_add(me->list, item);
    uint64_t buf = 1;
    write(me->writefd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(me->queue_mutex));

}
Functor Evfdq_remove(EvfdQueueRef athis)
{
    EvfQueuePtr me = (EvfQueuePtr)athis;
    pthread_mutex_lock(&(me->queue_mutex));
    Functor op = functor_list_remove(me->list);
    uint64_t buf;
    int nread = read(me->readfd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(me->queue_mutex));
    // remember to read from the pipe to clear the event
    return op;
}
