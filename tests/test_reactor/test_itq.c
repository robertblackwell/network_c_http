#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <string.h>

#include <c_http/unittest.h>
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>
/**
 * The purpose of this test is to demonstrate the function of inter thread queues.
 *
 * Runs two threads A and B.
 * Thread A has a reactor or run loop
 * Thread B is just any old thread.
 *  -   but it has a reference to A's reactor
 *  -   uses that to send data to thread A via A's reactor
 *
 */
#define QRDR_TAG     "ITQ_QRDR"
#define QWRT_TAG     "ITQ_QWRT"

#define QREADER_DECLARE_TAG DECLARE_TAG(QRDR_TAG)
#define QREADER_CHECK_TAG(p) CHECK_TAG(QRDR_TAG, p)
#define QREADER_SET_TAG(p) SET_TAG(QRDR_TAG, p)

#define QWRITER_DECLARE_TAG DECLARE_TAG(QWRT_TAG)
#define QWRITER_CHECK_TAG(p)      CHECK_TAG(QWRT_TAG, p)
#define QWRITER_SET_TAG(p)  SET_TAG(QWRT_TAG, p)

typedef struct QReader_s {
    QREADER_DECLARE_TAG;
    ReactorRef   _reactor_ref;
    int count;
    int expected_count;
} QReader, *QReaderRef;


QReaderRef qreader_new(int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    QREADER_SET_TAG(this);
    this->expected_count = expected_count;
    this->count = 0;
    this->_reactor_ref = rtor_reactor_new();
    XR_REACTOR_CHECK_TAG(this->_reactor_ref)
    rtor_reactor_enable_interthread_queue(this->_reactor_ref);
    return this;
}

void qreader_free(QReaderRef rdrref)
{
    QREADER_SET_TAG(rdrref);
    rtor_free(rdrref->_reactor_ref);
    free(rdrref);
}
ReactorRef qreader_get_reactor(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref;
}
EvfdQueueRef  qreader_get_evqueue(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref->interthread_queue_ref;
}
RtorWQueueRef qreader_get_queue_watcher(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref->interthread_queue_watcher_ref;
}

void qreader_post(QReaderRef rdrref, PostableFunction f, void* arg)
{
    ReactorRef rx = qreader_get_reactor(rdrref);
    rtor_reactor_post(rx, f, arg);
}

typedef struct QWriter_s {
    QWRITER_DECLARE_TAG;
    QReaderRef   qrdr_ref;
    EvfdQueueRef queue;
    int count_max;
    int count;
} QWriter, *QWriterRef;

QWriterRef QWriter_new(QReaderRef qrdr, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    QWRITER_SET_TAG(this);
    this->qrdr_ref = qrdr;
    this->queue = qreader_get_evqueue(this->qrdr_ref);
    this->count_max = max;
    this->count = 0;
}

void QWriter_free(QWriterRef this)
{
    qreader_free(this->qrdr_ref);
    free(this);
}

void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    QREADER_CHECK_TAG(q_rdr_ctx)
    ReactorRef rtor_ref = qreader_get_reactor(q_rdr_ctx);
    XR_REACTOR_CHECK_TAG(rtor_ref)
    RtorWQueueRef qw = qreader_get_queue_watcher(q_rdr_ctx);
    XR_WQUEUE_CHECK_TAG(qw)
    rtor_reactor_run(rtor_ref, -1);
}
void writers_postable_func(ReactorRef rtor_ref, void* arg)
{
    RtorWQueueRef wqref = (RtorWQueueRef)arg;
    XR_WQUEUE_CHECK_TAG(wqref)
    QWriterRef wrtref = (QWriterRef)wqref->queue_event_handler_arg;
    QWRITER_CHECK_TAG(wrtref);
    printf("writers postable func arg: %p rdrref:%p count:%d max_count:%d\n", arg, wrtref, wrtref->count, wrtref->count_max);
    if (wrtref->count >= wrtref->count_max) {
        printf("writer is done\n");
        rtor_wqueue_deregister(wqref);
    }
}
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    ReactorRef rtor_ref = qreader_get_reactor(wrtr->qrdr_ref);
    QWRITER_CHECK_TAG(wrtr)
    for(long i = 0; i <= wrtr->count_max; i++) {
        usleep(500000);
        wrtr->count = i;
        printf("writer thread  i: %ld\n", i);
        /**
         * This is the big test - send to a reactor on an other thread
         */
        rtor_reactor_interthread_post(rtor_ref, &writers_postable_func, (void *) wrtr);
    }
}

int test_itq()
{
    QReaderRef rdr = qreader_new(10);
    QWriterRef wrtr = QWriter_new(rdr, 10);

    pthread_t rdr_thread;
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    UT_TRUE(wrtr->count == wrtr->count_max);
    return 0;
}

int main()
{
    UT_ADD(test_itq);
    int rc = UT_RUN();
    return rc;
}
