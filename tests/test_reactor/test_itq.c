#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>

#include <sys/epoll.h>
#include <c_http/unittest.h>
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>


typedef struct QReader_s {
    ReactorRef   _reactor_ref;
//    EvfdQueueRef _queue;
//    WQueueRef    _queue_watcher_ref;
    int count;
    int expected_count;
} QReader, *QReaderRef;


QReaderRef QReader_new(int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    this->expected_count = expected_count;
    this->count = 0;
    this->_reactor_ref = rtor_new();
    return this;
}
ReactorRef Qreader_get_reactor(QReaderRef qr)
{
    return qr->_reactor_ref;
}
EvfdQueueRef  QReader_get_evqueue(QReaderRef qr)
{
    return qr->_reactor_ref->interthread_queue_ref;
}
WQueueRef QReader_get_queue_watcher(QReaderRef qr)
{
    return qr->_reactor_ref->interthread_queue_watcher_ref;
}
void QReader_dispose(QReaderRef this)
{
    free(this);
}

void QReader_post(QReaderRef rdrref, PostableFunction f, void* arg)
{
    ReactorRef rx = Qreader_get_reactor(rdrref);
    rtor_post(rx, f, arg);
}

typedef struct QWriter_s {
    EvfdQueueRef queue;
    int count_max;
    int count;
} QWriter, *QWriterRef;

QWriterRef QWriter_new(EvfdQueueRef queue, int max)
{
    QWriterRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->count_max = max;
    this->count = 0;
}

void QWriter_dispose(QReaderRef this)
{
    free(this);
}

void postable_function(void* arg)
{

}


void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    ReactorRef rtor_ref = Qreader_get_reactor(q_rdr_ctx);
    WQueueRef qw = QReader_get_queue_watcher(q_rdr_ctx);
    rtor_run(rtor_ref, -1);
}
void writers_postable_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    printf("writers postable func arg: %p wrtr:%p count:%d max_count:%d\n", arg, wrtr, wrtr->count, wrtr->count_max);
    if (wrtr->count >= wrtr->count_max) {
        printf("writer is done\n");
    }
}
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    for(long i = 0; i <= wrtr->count_max; i++) {
        sleep(2);
        wrtr->count = i;
        FunctorRef f = Functor_new(&writers_postable_func, (void*)wrtr);
        printf("writer thread f: %p i: %ld\n", f, i);
        Evfdq_add(wrtr->queue, (void*)f);
    }
}

int test_q()
{
    EvfdQueueRef queue = Evfdq_new();
    ReactorRef reactor_ref = rtor_new();
    QReaderRef rdr = QReader_new(10);
    QWriterRef wrtr = QWriter_new(QReader_get_evqueue(rdr), 10);

    pthread_t rdr_thread;
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    UT_TRUE(rdr->expected_count == rdr->count);
    UT_TRUE(rdr->count == wrtr->count_max);
    return 0;
}

int main()
{
    UT_ADD(test_q);
    int rc = UT_RUN();
    return rc;
}
