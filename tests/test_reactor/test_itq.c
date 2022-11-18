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
    EvfdQueueRef _queue;
    WQueueRef    _queue_watcher_ref;
    int count;
    int expected_count;
} QReader, *QReaderRef;

void QReaderHandler(WQueueRef qw, void* ctx, uint64_t event);
#define QINREACTOR 1
QReaderRef QReader_new2(int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    this->expected_count = expected_count;
    this->count = 0;

#if QINREACTOR == 1
    this->_reactor_ref = rtor_new();
#if !REGISTER_WQUEUE_REACTOR
    WQueueRef qwref = this->_reactor_ref->interthread_queue_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qwref, QReaderHandler, (void*)this, interest);
#endif
#elif QINREACTOR == 2
    this->_reactor_ref = rtor_new();
    WQueueRef qwref = this->_reactor_ref->interthread_queue_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qwref, QReaderHandler, (void*)this, interest);
#elif QINREACTOR == 3
    this->_reactor_ref = rtor_new();
    this->_reactor_ref->interthread_queue_ref = Evfdq_new();
    this->_reactor_ref->interthread_queue_watcher_ref = WQueue_new(this->_reactor_ref, this->_reactor_ref->interthread_queue_ref);
    WQueueRef qwref = this->_reactor_ref->interthread_queue_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qwref, QReaderHandler, (void*)this, interest);
#else
    this->_reactor_ref = rtor_new();
    this->_queue = Evfdq_new();
    this->_queue_watcher_ref = WQueue_new(this->_reactor_ref, this->_queue);
    WQueueRef qwref = this->_queue_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qwref, QReaderHandler, (void*)this, interest);
#endif
    return this;
}
ReactorRef Qreader_get_reactor(QReaderRef qr)
{
#if QINREACTOR
    return qr->_reactor_ref;
#else
    return qr->_reactor_ref;
#endif
}
EvfdQueueRef  QReader_get_evqueue(QReaderRef qr)
{
#if QINREACTOR
    return qr->_reactor_ref->interthread_queue_ref;
#else
    return qr->_queue;
#endif
}
WQueueRef QReader_get_queue_watcher(QReaderRef qr)
{
#if QINREACTOR
    return qr->_reactor_ref->interthread_queue_watcher_ref;
#else
    return qr->_queue_watcher_ref;
#endif
}


QReaderRef QReader_new(ReactorRef reactor_ref, EvfdQueueRef queue, int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    this->_reactor_ref = reactor_ref;
    this->_queue = queue;
    this->_queue_watcher_ref = WQueue_new(reactor_ref, queue);
    this->expected_count = expected_count;
    this->count = 0;
    return this;
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
} QWriter, *QSyncWriterRef;

QSyncWriterRef QWriter_new(EvfdQueueRef queue, int max)
{
    QSyncWriterRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->count_max = max;
}

void QWriter_dispose(QReaderRef this)
{
    free(this);
}

void postable_function(void* arg)
{

}

void QReaderHandler(WQueueRef qw, void* ctx, uint64_t event)
{
    QReaderRef rdr = (QReaderRef)ctx;

    EvfdQueueRef queue = QReader_get_evqueue(rdr);
    void* queue_data = Evfdq_remove(queue);
    FunctorRef fref = (FunctorRef)queue_data;
    void* pf = fref->f;
    long d = (long)fref->arg;
    printf("Q Handler received f: %p d: %ld count: %d\n", pf, d, rdr->count);
    bool x = (long)queue_data == (long)rdr->count;
//    assert(x);
//    fref->f(fref->arg);
    rtor_post(Qreader_get_reactor(rdr), fref->f, fref->arg);
    rdr->count++;
    if (rdr->count >= rdr->expected_count) {
        WQueue_deregister(qw);
    }

}

void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
//    ReactorRef rtor_ref = rtor_new();
//    WQueueRef qw = WQueue_new(rtor_ref, q_rdr_ctx->queue);

    ReactorRef rtor_ref = Qreader_get_reactor(q_rdr_ctx);
    WQueueRef qw = QReader_get_queue_watcher(q_rdr_ctx);

//    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
//    WQueue_register(qw, QReaderHandler, arg, interest);
    rtor_run(rtor_ref, -1);
}
void writers_postable_func(void* arg)
{
    printf("writers postable func arg: %p\n", arg);
}
void* writer_thread_func(void* arg)
{
    QSyncWriterRef wrtr = (QSyncWriterRef)arg;
    for(long i = 0; i < 10; i++) {
        sleep(2);
        FunctorRef f = Functor_new(&writers_postable_func, (void*)i);
        printf("writer thread f: %p i: %ld\n", f, i);
        Evfdq_add(wrtr->queue, (void*)f);
    }
}

int test_q()
{
    EvfdQueueRef queue = Evfdq_new();
    ReactorRef reactor_ref = rtor_new();
    QReaderRef rdr = QReader_new2(10);
    QSyncWriterRef wrtr = QWriter_new(QReader_get_evqueue(rdr), 10);

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
