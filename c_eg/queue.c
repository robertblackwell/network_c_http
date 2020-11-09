//
//  main.cpp
//  Pixie-XC
//
//  Created by ROBERT BLACKWELL on 12/27/15.
//  Copyright © 2015 Blackwellapps. All rights reserved.
//
#include <c_eg/queue.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include <c_eg/constants.h>
#include <c_eg/alloc.h>

#define QUEUE_CIRCULAR

struct Queue_s {
    SocketFD        q[QUEUE_MAX_SIZE];
    uint32_t        size;
    uint32_t        max_size;
    /**
     * Used to make the array into a circular buffer
     */
    uint32_t        head; // points at the next available slot
    uint32_t        tail; //
    bool            full;

    pthread_mutex_t queue_mutex;
    pthread_cond_t  not_empty_cv;
    pthread_cond_t  not_full_cv;
};

static uint32_t q_size(QueueRef q)
{
    uint32_t sz;
    if(q->full) {
        sz = q->max_size;
    } else {
        if(q->head > q->tail) {
            sz = q->head - q->tail;
        } else {
            sz = q->max_size + q->head - q->tail;
        }
    }
    return sz;
}

static void advance_pointer(QueueRef q)
{
    if(q->full)
    {
        q->tail = (q->tail + 1) % q->max_size;
    }

    q->head = (q->head + 1) % q->max_size;
    q->full = (q->head == q->tail);
}
static void retreat_pointer(QueueRef q)
{
    q->full = false;
    q->tail = (q->tail + 1) % q->max_size;
}
static bool q_full(QueueRef qref)
{
    assert(qref);

    return qref->full;
}

static bool q_empty(QueueRef qref)
{
    assert(qref);

    return (!qref->full && (qref->head == qref->tail));
}

QueueRef Queue_new()
{
    QueueRef q = (QueueRef)eg_alloc(sizeof(Queue));
    q->max_size = QUEUE_MAX_SIZE;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    q->full = false;
    pthread_mutex_init(&(q->queue_mutex), NULL);
    pthread_cond_init(&(q->not_empty_cv), NULL);
    pthread_cond_init(&(q->not_full_cv), NULL);
    return q;
}    
void Queue_free(QueueRef* qref_ptr)
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
    while( q_empty(qref) ){
        pthread_cond_wait(&(qref->not_empty_cv), &(qref->queue_mutex));
    }
    SocketFD r = qref->q[qref->tail];
    retreat_pointer(qref);

    if (qref->size == QUEUE_MAX_SIZE - 1) {
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
    if (qref->size == QUEUE_MAX_SIZE - 1) {
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
    }
    qref->q[qref->head] = sock;
    advance_pointer(qref);
    qref->size = q_size(qref);
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
    printf("Queue_add: %d\n", qref->size);
    pthread_mutex_unlock(&(qref->queue_mutex));
}

