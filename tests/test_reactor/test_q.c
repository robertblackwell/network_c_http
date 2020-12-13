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
#include <c_http/dsl/utils.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/w_queue.h>
#include <c_http/runloop/evfd_queue.h>

typedef struct QReader_s {
    EvfdQueueRef queue;
    int count;
    int expected_count;
} QReader, *QReaderRef;


QReaderRef QReader_new(EvfdQueueRef queue, int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->expected_count = expected_count;
    this->count = 0;
}
void QReader_free(QReaderRef this)
{
    free(this);
}

typedef struct QWriter_s {
    EvfdQueueRef queue;
    int count_max;
} QWriter, *QWriterRef;

QWriterRef QWriter_new(EvfdQueueRef queue, int max)
{
    QWriterRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->count_max = max;
}

void QWriter_free(QReaderRef this)
{
    free(this);
}

void QReaderHandler(WQueueRef qw, void* ctx, uint64_t event)
{
    QReaderRef rdr = (QReaderRef)ctx;

    EvfdQueueRef queue = rdr->queue;
    void* queue_data = Evfdq_remove(queue);

    printf("Q Handler received %p count: %d\n", queue_data, rdr->count);
    bool x = (long)queue_data == (long)rdr->count;
    assert(x);

    rdr->count++;
    if (rdr->count >= rdr->expected_count) {
        WQueue_deregister(qw);
    }

}

void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    XrReactorRef rtor_ref = XrReactor_new();
    WQueueRef qw = WQueue_new(rtor_ref, q_rdr_ctx->queue);
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qw, QReaderHandler, arg, interest);
    XrReactor_run(rtor_ref, -1);
}
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    for(long i = 0; i < 10; i++) {
        sleep(2);
        Evfdq_add(wrtr->queue, (void*)i);
    }
}

int test_q()
{
    EvfdQueueRef queue = Evfdq_new();
    QReaderRef rdr = QReader_new(queue, 10);
    QWriterRef wrtr = QWriter_new(queue, 10);

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
