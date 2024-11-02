
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <string.h>

#include <rbl/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
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
#define QReader_TAG     "ITQ_QRDR"
#define QWriter_TAG     "ITQ_QWRT"

typedef struct QReader_s {
    RBL_DECLARE_TAG;
    RunloopRef   _reactor_ref;
    int count;
    int expected_count;
    EventfdQueueRef         evqueue_ref;
    RunloopQueueWatcherRef  queue_watcher_ref;
} QReader, *QReaderRef;

void qcallback(RunloopQueueWatcherRef qref, uint64_t event)
{
    printf("queue handler\n");
}


QReaderRef qreader_new(int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    RBL_SET_TAG(QReader_TAG, this);
    this->expected_count = expected_count;
    this->count = 0;
    this->_reactor_ref = runloop_new();
    REACTOR_CHECK_TAG(this->_reactor_ref)
    this->evqueue_ref = runloop_eventfd_queue_new();
    this->queue_watcher_ref = runloop_queue_watcher_new(this->_reactor_ref, this->evqueue_ref);
    runloop_queue_watcher_register(this->queue_watcher_ref, qcallback, this);

//    runloop_ref->interthread_queue_watcher_ref = runloop_queue_watcher_new(runloop_ref,
//                                                                           runloop_ref->interthread_queue_ref);
//    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
//    runloop_queue_watcher_register(runloop_ref->interthread_queue_watcher_ref, &interthread_queue_handler,
//                                   (void *) runloop_ref->interthread_queue_ref);
//
//    runloop_enable_interthread_queue(this->_reactor_ref);
    return this;
}

void qreader_free(QReaderRef rdrref)
{
    RBL_CHECK_TAG(QReader_TAG, rdrref);
    runloop_free(rdrref->_reactor_ref);
    free(rdrref);
}
RunloopRef qreader_get_reactor(QReaderRef qr)
{
    RBL_CHECK_TAG(QReader_TAG, qr);
    return qr->_reactor_ref;
}
EventfdQueueRef  qreader_get_evqueue(QReaderRef qr)
{
    RBL_CHECK_TAG(QReader_TAG, qr);
    return qr->_reactor_ref->interthread_queue_ref;
}
RunloopQueueWatcherRef qreader_get_queue_watcher(QReaderRef qr)
{
    RBL_CHECK_TAG(QReader_TAG, qr);
    return qr->_reactor_ref->interthread_queue_watcher_ref;
}

void qreader_post(QReaderRef rdrref, PostableFunction f, void* arg)
{
    RunloopRef rx = qreader_get_reactor(rdrref);
    runloop_post(rx, f, arg);
}

typedef struct QWriter_s {
    RBL_DECLARE_TAG;
    QReaderRef   qrdr_ref;
    EventfdQueueRef queue;
    int count_max;
    int count;
} QWriter, *QWriterRef;

QWriterRef QWriter_new(QReaderRef qrdr, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    RBL_SET_TAG(QWriter_TAG, this);
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
void timercb(RunloopTimerRef timer, uint64_t event)
{
    printf("readthread timer ccb\n");
}
void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    RBL_CHECK_TAG(QReader_TAG, q_rdr_ctx)
    RunloopRef runloop_ref = qreader_get_reactor(q_rdr_ctx);
    RunloopTimerRef timerref = runloop_timer_set(runloop_ref, &timercb, NULL, 2000, true);
    REACTOR_CHECK_TAG(runloop_ref)
    RunloopQueueWatcherRef qw = qreader_get_queue_watcher(q_rdr_ctx);


    WQUEUE_CHECK_TAG(qw)
    runloop_run(runloop_ref, -1);
    return NULL;
}
void writers_postable_func(RunloopRef runloop_ref, void* arg)
{
    RunloopQueueWatcherRef wqref = (RunloopQueueWatcherRef)arg;
    WQUEUE_CHECK_TAG(wqref)
    QWriterRef wrtref = (QWriterRef)wqref->queue_event_handler_arg;
    RBL_CHECK_TAG(QWriter_TAG, wrtref);
    printf("writers postable func arg: %p rdrref:%p count:%d max_count:%d\n", arg, wrtref, wrtref->count, wrtref->count_max);
    if (wrtref->count >= wrtref->count_max) {
        printf("writer is done\n");
        runloop_queue_watcher_deregister(wqref);
    }
}
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    RunloopRef runloop_ref = qreader_get_reactor(wrtr->qrdr_ref);
    RBL_CHECK_TAG(QWriter_TAG, wrtr)
    for(long i = 0; i <= wrtr->count_max; i++) {
        usleep(500000);
        wrtr->count = (int)i;
        printf("writer thread  i: %ld\n", i);
        /**
         * This is the big test - send to a reactor on an other thread
         */
        runloop_interthread_post(runloop_ref, &writers_postable_func, (void *) wrtr);
    }
    return NULL;
}
/**
 * start a reader thread and a writer thread.
 * The writer thread periodically posts or schedules a call back to run
 * on a runloop owned by the reader thread.
 * That callback uses a context to count the number of times it is called
 */
int test_itq()
{
    QReaderRef rdr = qreader_new(10);
    QWriterRef wrtr = QWriter_new(rdr, 10);

    pthread_t rdr_thread;
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
//    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
//    pthread_join(wrtr_thread, NULL);
    UT_TRUE((wrtr->count == wrtr->count_max));
    return 0;
}

int main()
{
    UT_ADD(test_itq);
    int rc = UT_RUN();
    return rc;
}
