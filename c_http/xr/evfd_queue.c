#define _GNU_SOURCE
#include <c_http/xr/evfd_queue.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/eventfd.h>

#include <c_http/xr/types.h>
#include <c_http/list.h>

// enables use of eventfd rather than two pipe trick
#define  C_HTTP_EFD_QUEUE

struct EvfdQueue_s {
    ListRef         list;
    pthread_mutex_t queue_mutex;
#ifdef C_HTTP_EFD_QUEUE
#else
    int             pipefds[2];
#endif
    int             readfd;
    int             writefd;
    int             id;
};
static void dealloc(void** p)
{
}
static void mk_fds(EvfdQueueRef this)
{
#ifdef C_HTTP_EFD_QUEUE
    int fd = eventfd(0, O_NONBLOCK | O_CLOEXEC);
    this->readfd = fd;
    this->writefd = fd;
    uint64_t buf;
    while(1) {
        int nread = read(fd, &buf, sizeof(buf));
        if (nread == -1) break;
    }
    assert(errno == EAGAIN);
#else
    pipe2(this->pipefds, O_NONBLOCK | O_CLOEXEC);
    this->readfd = this->pipefds[0];
    this->writefd = this->pipefds[1];
#endif
}
void Evfdq_init(EvfdQueueRef this)
{
    this->list = List_new(&dealloc);
    pthread_mutex_init(&(this->queue_mutex), NULL);
    mk_fds(this);
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
int Evfdq_readfd(EvfdQueueRef this)
{
    return this->readfd;
}
void Evfdq_add(EvfdQueueRef this, void* item)
{
    pthread_mutex_lock(&(this->queue_mutex));
    List_add_back(this->list, item);
    XR_PRINTF("Queue_add: %d\n", List_size(this->list));
    uint64_t buf = 1;
    write(this->writefd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(this->queue_mutex));

}
void* Evfdq_remove(EvfdQueueRef this)
{
    pthread_mutex_lock(&(this->queue_mutex));
    void* op = List_remove_first(this->list);
    uint64_t buf;
    int nread = read(this->readfd, &buf, sizeof(buf));
    pthread_mutex_unlock(&(this->queue_mutex));
    XR_PRINTF("Queue_pop: socket is %ld nread: %d buf : %ld\n", (long)op, nread, buf);
    // remember to read from the pipe to clear the event
    return op;
}

