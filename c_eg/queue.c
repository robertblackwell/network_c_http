//
//  main.cpp
//  Pixie-XC
//
//  Created by ROBERT BLACKWELL on 12/27/15.
//  Copyright © 2015 Blackwellapps. All rights reserved.
//
#include <c_eg/queue.h>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include <c_eg/constants.h>
#include <c_eg/alloc.h>

struct Queue_s {
    int              q[QUEUE_MAX_SIZE];
    int              size;
    int              max_size;
    pthread_mutex_t  queue_mutex;
    pthread_cond_t   not_empty_cv;
    pthread_cond_t   not_full_cv;
};


Queue* Queue_new()
{
    Queue* q = (Queue*)eg_alloc(sizeof(Queue));
    q->max_size = QUEUE_MAX_SIZE;
    q->size = 0;
    pthread_mutex_init(&(q->queue_mutex), NULL);
    pthread_cond_init(&(q->not_empty_cv), NULL);
    pthread_cond_init(&(q->not_full_cv), NULL);
    return q;
}    
void Queue_free(Queue** qref_ptr)
{
    Queue* qref = * qref_ptr;
    pthread_cond_destroy(&(qref->not_full_cv));
    pthread_cond_destroy(&(qref->not_empty_cv));
    pthread_mutex_destroy(&(qref->queue_mutex));
    free((void*)qref);
    *qref_ptr = NULL;
}
    
int Queue_remove(Queue* qref)
{
    pthread_mutex_lock(&(qref->queue_mutex));
    while( qref->size == 0 ){
        pthread_cond_wait(&(qref->not_empty_cv), &(qref->queue_mutex));
    }
    int r = qref->q[0];
    // shuffle down
    for(int i = 0; i < qref->size - 1; i++) {
        // printf("In remove i : %d q[i] %d  q[i-1] %d\n", i, qref->q[i], qref->q[i-1]);
        qref->q[i] = qref->q[i+1];
    }
    // clear the entry above data
    qref->q[qref->size - 1] = 0;
    qref->size -= 1;

    if (qref->size == QUEUE_MAX_SIZE - 1) {
        pthread_cond_broadcast(&(qref->not_full_cv));
    }
    pthread_mutex_unlock(&(qref->queue_mutex));
    return r;
}
    
void Queue_add(Queue* qref, int a)
{
    pthread_mutex_lock(&(qref->queue_mutex));
    // in here put a wait for space on the queue
    while(qref->size == qref->max_size) {
        pthread_cond_wait(&(qref->not_full_cv), &(qref->queue_mutex));
    }
    qref->q[qref->size] = a;
    qref->size++;
    if( qref->size == 1 ){
        // then it was empty before this add
        pthread_cond_broadcast(&(qref->not_empty_cv));
    }
    pthread_mutex_unlock(&(qref->queue_mutex));
}

