#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//
// This is a simple example of using a mutex and 2 condition variables to
// sync a single writer and multiple readers interacting with a bounded (fixed max size) queue
//
// in this toy example a queue is simulated by an int counter n_resource
//

int n_resource;
pthread_cond_t rdr_cvar;
pthread_cond_t wrtr_cvar;
pthread_mutex_t mutex;

void reader(void* data)
{
    long id = (long)data;
    for(;;) {

        pthread_mutex_lock(&mutex);
        while (n_resource <= 0) {
            pthread_cond_wait(&rdr_cvar, &mutex);
        }
        printf("Reader %ld n_resource = %d\n", id, n_resource);
        --n_resource;
        // if there are still things to read - singla one reader
        if(n_resource > 0) {
            pthread_cond_signal(&rdr_cvar);
        }
        // if there is space for the writer to add another signal the writer
        if(n_resource < 10) {
            pthread_cond_signal(&wrtr_cvar);
        }
        pthread_mutex_unlock(&mutex);
    }
}
void writer(void* data)
{
    for(;;) {

        pthread_mutex_lock(&mutex);
        printf("Writer before while n_resource %d \n", n_resource);
        while (n_resource > 10) {
            pthread_cond_wait(&wrtr_cvar, &mutex);
        }
        printf("Writer after while n_resource %d \n", n_resource);

        ++n_resource;
        // if there is something for a reader to read signal one of the readers.
        if(n_resource > 0) {
            pthread_cond_signal(&rdr_cvar);
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    pthread_t rdr_thread_1;
    pthread_t rdr_thread_2;
    pthread_t wrtr_thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&rdr_cvar, NULL);
    pthread_cond_init(&wrtr_cvar, NULL);
    pthread_create(&rdr_thread_1, NULL, &reader, (void*)1L);
    pthread_create(&rdr_thread_2, NULL, &reader, (void*)2L);
    pthread_create(&wrtr_thread, NULL, &writer, NULL);
    pthread_join(wrtr_thread, NULL);
    pthread_join(rdr_thread_1, NULL);
    pthread_join(rdr_thread_2, NULL);
}