
#include <http_in_c/common/queue.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <http_in_c/common/alloc.h>
#define ENABLE_LOGX
#include <http_in_c/logger.h>

#define QUEUE_DEFAULT_SIZE 10000
#define QUEUE_CIRCULAR
#define QUEUE_DYN_CAPACITY

struct Queue_s {
#ifdef QUEUE_DYN_CAPACITY
    SocketFD*         q;
#else
    SocketFD        q[QUEUE_MAX_SIZE];
#endif
    uint32_t        size;
    uint32_t        max_size;
    /**
     * Used to make the array into a circular buffer
     */
    uint32_t        next;  // points at the next entry to be used
    uint32_t        first; // points at the first used entry
    bool            full;

    pthread_mutex_t queue_mutex;
    pthread_cond_t  not_empty_cv;
    pthread_cond_t  not_full_cv;
};
static uint32_t q_size(QueueRef q)
{
    if(q->first <= q->next) {
        return q->next - q->first;
    } else {
        return (q->next + q->max_size) - q->first;
    }
}
static bool is_full(QueueRef q)
{
    return (q->first == q->next) && (q->size > 0);
}
static bool is_empty(QueueRef q)
{
    return((q->first == q->next) && (q->size == 0));
}
static void advance_next_pointer(QueueRef q)
{
    assert(!is_full(q));

    q->next = (q->next + 1) % q->max_size;
    q->full = is_full(q);
}
// removing an item
static void advance_first_pointer(QueueRef q)
{
    assert(!is_empty(q));
    q->first = (q->first + 1) % q->max_size;
    q->full = is_full(q);
}
static bool q_full(QueueRef qref)
{
    assert(qref);
    qref->full = is_full(qref);
    return qref->full;
}

QueueRef Queue_new_with_capacity(size_t capacity)
{
    QueueRef q = (QueueRef)eg_alloc(sizeof(Queue));
    q->max_size = capacity;
    q->q = malloc(sizeof(SocketFD) * q->max_size);
    q->size = 0;
    q->first = 0;
    q->next = 0;
    q->full = false;
    pthread_mutex_init(&(q->queue_mutex), NULL);
    pthread_cond_init(&(q->not_empty_cv), NULL);
    pthread_cond_init(&(q->not_full_cv), NULL);
    return q;
}

QueueRef Queue_new()
{
    QueueRef q = (QueueRef)eg_alloc(sizeof(Queue));
    q->max_size = QUEUE_DEFAULT_SIZE;
    q->q = malloc(sizeof(SocketFD) * q->max_size);
    q->size = 0;
    q->first = 0;
    q->next = 0;
    q->full = false;
    pthread_mutex_init(&(q->queue_mutex), NULL);
    pthread_cond_init(&(q->not_empty_cv), NULL);
    pthread_cond_init(&(q->not_full_cv), NULL);
    return q;
}    
void Queue_dispose(QueueRef* qref_ptr)
{
    QueueRef qref = * qref_ptr;
    pthread_cond_destroy(&(qref->not_full_cv));
    pthread_cond_destroy(&(qref->not_empty_cv));
    pthread_mutex_destroy(&(qref->queue_mutex));
    free((void*)qref);
    *qref_ptr = NULL;
}
    
SocketFD Queue_remove(QueueRef qref)
{
    pthread_mutex_lock(&(qref->queue_mutex));

#ifdef QUEUE_CIRCULAR
    while( is_empty(qref) ){
        pthread_cond_wait(&(qref->not_empty_cv), &(qref->queue_mutex));
    }
    SocketFD r = qref->q[qref->first];
    advance_first_pointer(qref);
    qref->size--;
    LOG_FMT("qref->size: %d qref->max_size %d\n", qref->size, qref->max_size);
    if (qref->size < qref->max_size) {
        pthread_cond_broadcast(&(qref->not_full_cv));
    }
#else
    while( qref->size == 0 ){
        pthread_cond_wait(&(qref->not_empty_cv), &(qref->queue_mutex));
    }    SocketFD r = qref->q[0];
    for(int i = 0; i < qref->size - 1; i++) {
        qref->q[i] = qref->q[i+1];
    }
    // clear the entry above data
    qref->q[qref->size - 1] = 0;
    qref->size -= 1;
    if (qref->size == qref->max_size - 1) {
        pthread_cond_broadcast(&(qref->not_full_cv));
    }
#endif

    pthread_mutex_unlock(&(qref->queue_mutex));
    return r;
}
    
void Queue_add(QueueRef qref, SocketFD sock)
{
    pthread_mutex_lock(&(qref->queue_mutex));
#ifdef QUEUE_CIRCULAR
    while(qref->size == qref->max_size) {
        pthread_cond_wait(&(qref->not_full_cv), &(qref->queue_mutex));
        LOG_FMT("Queue_add:wait not_full qref->size: %d qref->max_size %d\n", qref->size, qref->max_size);
    }
    qref->q[qref->next] = sock;
    advance_next_pointer(qref);
    qref->size++;
    if( qref->size == 1 ){
        pthread_cond_broadcast(&(qref->not_empty_cv));
    }
#else
    while(qref->size == qref->max_size) {
        pthread_cond_wait(&(qref->not_full_cv), &(qref->queue_mutex));
    }
    qref->q[qref->size] = sock;
    qref->size++;
    if( qref->size == 1 ){
        pthread_cond_broadcast(&(qref->not_empty_cv));
    }
#endif
    LOG_FMT("Queue_add: %d\n", qref->size);
    pthread_mutex_unlock(&(qref->queue_mutex));
}
size_t Queue_size(QueueRef this)
{
    return this->size;
}
size_t Queue_capacity(QueueRef this)
{
    return this->max_size;
}

