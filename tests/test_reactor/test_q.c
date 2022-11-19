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
    EvfdQueueRef queue;
    int count;
    int expected_count;
} QReader, *QSyncReaderRef;


QSyncReaderRef QReader_new(EvfdQueueRef queue, int expected_count)
{
    QSyncReaderRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->expected_count = expected_count;
    this->count = 0;
}
void QReader_dispose(QSyncReaderRef this)
{
    free(this);
}

typedef struct QWriter_s {
    EvfdQueueRef queue;
    int count_max;
} QWriter, *QSyncWriterRef;

QSyncWriterRef QWriter_new(EvfdQueueRef queue, int max)
{
    QSyncWriterRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->count_max = max;
}

void QWriter_dispose(QSyncReaderRef this)
{
    free(this);
}

void QReaderHandler(RtorWQueueRef qw, uint64_t event)
{
    void* ctx = qw->queue_event_handler_arg;
    QSyncReaderRef rdr = (QSyncReaderRef)ctx;

    EvfdQueueRef queue = rdr->queue;
    Functor queue_data = Evfdq_remove(queue);

    printf("Q Handler received %p count: %d\n", &queue_data, rdr->count);
    bool x = (long)queue_data.arg == (long)rdr->count;
    assert(x);

    rdr->count++;
    if (rdr->count >= rdr->expected_count) {
        rtor_wqueue_deregister(qw);
    }

}

void* reader_thread_func(void* arg)
{
    QSyncReaderRef q_rdr_ctx = (QSyncReaderRef)arg;
    ReactorRef rtor_ref = rtor_reactor_new();
    RtorWQueueRef qw = rtor_wqueue_new(rtor_ref, q_rdr_ctx->queue);
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    rtor_wqueue_register(qw, QReaderHandler, arg, interest);
    rtor_reactor_run(rtor_ref, -1);
}
void* writer_thread_func(void* arg)
{
    QSyncWriterRef wrtr = (QSyncWriterRef)arg;
    for(long i = 0; i < 10; i++) {
        usleep(500000);
        Functor func = {.f = (void*)i, .arg = (void*) i};
        Evfdq_add(wrtr->queue, func);
    }
}

int test_q()
{
    EvfdQueueRef queue = Evfdq_new();
    QSyncReaderRef rdr = QReader_new(queue, 10);
    QSyncWriterRef wrtr = QWriter_new(queue, 10);

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
