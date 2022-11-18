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


QReaderRef QReader_new(int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    QREADER_SET_TAG(this);
    this->expected_count = expected_count;
    this->count = 0;
    this->_reactor_ref = rtor_new();
    XR_REACTOR_CHECK_TAG(this->_reactor_ref)
    rtor_enable_interthread_queue(this->_reactor_ref);

    return this;
}
ReactorRef Qreader_get_reactor(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref;
}
EvfdQueueRef  QReader_get_evqueue(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref->interthread_queue_ref;
}
WQueueRef QReader_get_queue_watcher(QReaderRef qr)
{
    QREADER_CHECK_TAG(qr);
    return qr->_reactor_ref->interthread_queue_watcher_ref;
}
void QReader_dispose(QReaderRef this)
{
    QREADER_CHECK_TAG(this);
    free(this);
}

void QReader_post(QReaderRef rdrref, PostableFunction f, void* arg)
{
    ReactorRef rx = Qreader_get_reactor(rdrref);
    rtor_post(rx, f, arg);
}

typedef struct QWriter_s {
    QWRITER_DECLARE_TAG;
    EvfdQueueRef queue;
    int count_max;
    int count;
} QWriter, *QWriterRef;

QWriterRef QWriter_new(EvfdQueueRef queue, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    QWRITER_SET_TAG(this);
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
    QREADER_CHECK_TAG(q_rdr_ctx)
    ReactorRef rtor_ref = Qreader_get_reactor(q_rdr_ctx);
    XR_REACTOR_CHECK_TAG(rtor_ref)
    WQueueRef qw = QReader_get_queue_watcher(q_rdr_ctx);
    XR_WQUEUE_CHECK_TAG(qw)
    rtor_run(rtor_ref, -1);
}
void writers_postable_func(void* arg)
{
    WQueueRef wqref = (WQueueRef)arg;
    XR_WQUEUE_CHECK_TAG(wqref)
    QWriterRef wrtref = (QWriterRef)wqref->queue_event_handler_arg;
    QWRITER_CHECK_TAG(wrtref);
    printf("writers postable func arg: %p rdrref:%p count:%d max_count:%d\n", arg, wrtref, wrtref->count, wrtref->count_max);
    if (wrtref->count >= wrtref->count_max) {
        printf("writer is done\n");
        WQueue_deregister(wqref);
    }
}
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    QWRITER_CHECK_TAG(wrtr)
    for(long i = 0; i <= wrtr->count_max; i++) {
        sleep(2);
        wrtr->count = i;
        FunctorRef f = Functor_new(&writers_postable_func, (void*)wrtr);
        printf("writer thread f: %p i: %ld\n", f, i);
        Evfdq_add(wrtr->queue, (void*)f);
    }
}

int test_itq()
{
    QReaderRef rdr = QReader_new(10);
    QWriterRef wrtr = QWriter_new(QReader_get_evqueue(rdr), 10);

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
