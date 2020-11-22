#define _GNU_SOURCE
#include <c_http/xr/evfd_queue.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <c_http/list.h>

struct EvfdQueue_s {
    ListRef         list;
    pthread_mutex_t queue_mutex;
    int             pipefds[2];
    int             readfd;
    int             writefd;
    int             id;
};
static void dealloc(void** p)
{
}
void Evfdq_init(EvfdQueueRef this)
{
    this->list = List_new(&dealloc);
    pthread_mutex_init(&(this->queue_mutex), NULL);
    pipe2(this->pipefds, O_NONBLOCK | O_CLOEXEC);
    this->readfd = this->pipefds[0];
    this->writefd = this->pipefds[1];
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
    printf("Queue_add: %d\n", List_size(this->list));
    uint64_t buf;
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
    printf("Queue_pop: socket is %ld\n", (long)op);
    // remember to read from the pipe to clear the event
    return op;
}

