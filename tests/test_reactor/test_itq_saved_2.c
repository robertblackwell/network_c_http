
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>

#include <sys/epoll.h>
#include <rbl/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>

ReactorRef global_reactor_ref = NULL;

void post_cb(void* arg)
{
    printf("final interthread_post_cb arg: %p\n", arg);
}

void interthread_post_cb(WQueueRef qw, void* ctx, uint64_t event)
{
    ReactorRef rx = WQueue_get_reactor(qw);
    printf("interthread_post_cb ctx: %p  reactor: %p\n", ctx, WQueue_get_reactor(qw));
    rtor_post(rx, &post_cb, NULL);
}

typedef struct QReader_s {
    EvfdQueueRef queue;
    ReactorRef   reactor_ref;
    WQueueRef    queue_watcher_ref;
    int count;
    int expected_count;
} QReader, *QSyncReaderRef;


QSyncReaderRef QReader_new(ReactorRef reactor_ref, EvfdQueueRef queueref, int expected_count)
{
    QSyncReaderRef this = malloc(sizeof(QReader));
    this->reactor_ref = reactor_ref;
    this->queue = queueref;
    this->queue_watcher_ref = WQueue_new(reactor_ref, queueref);
    this->expected_count = expected_count;
    this->count = 0;
}
void QReader_dispose(QSyncReaderRef this)
{
    free(this);
}

typedef struct QWriter_s {
    ReactorRef   reactor_ref;
    EvfdQueueRef queue;
    int count_max;
} QWriter, *QSyncWriterRef;

QSyncWriterRef QWriter_new(ReactorRef reactor_ref, EvfdQueueRef queueref, int max)
{
    QSyncWriterRef this = malloc(sizeof(QReader));
    this->reactor_ref = reactor_ref;
    this->queue = queueref;
    this->count_max = max;
}

void QWriter_dispose(QSyncReaderRef this)
{
    free(this);
}

void QReaderHandler(WQueueRef qw, void* ctx, uint64_t event)
{
    printf("probably should not be here\n");
#if 1
    QSyncReaderRef rdr = (QSyncReaderRef)ctx;

    EvfdQueueRef queue = rdr->queue;
    void* queue_data = Evfdq_remove(queue);

    printf("Q Handler received %p count: %d\n", queue_data, rdr->count);
    bool x = (long)queue_data == (long)rdr->count;
    assert(x);

    rdr->count++;
    if (rdr->count >= rdr->expected_count) {
        WQueue_deregister(qw);
    }
#endif
}
void timer_callback(RtorTimerRef t, void* arg, uint64_t event)
{
    printf("timercallback\n");
}
void* reader_thread_func(void* arg)
{

    QSyncReaderRef q_rdr_ctx = (QSyncReaderRef)arg;
    ReactorRef rtor_ref = rtor_new();
    RtorTimerRef tw_2 = rtor_timer_new(rtor_ref, &timer_callback, NULL, 50000, true);

    global_reactor_ref = rtor_ref;

    WQueueRef qw = q_rdr_ctx->queue_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    WQueue_register(qw,  &QReaderHandler, arg, interest);
    rtor_run(rtor_ref, -1);
    printf("reader complete \n");
}
void postable_function(void* arg)
{
    printf("postable function\n");
}
void* writer_thread_func(void* arg)
{
    sleep(2);
    QSyncWriterRef wrtr = (QSyncWriterRef)arg;
    ReactorRef rx = wrtr->reactor_ref;
    EvfdQueueRef wrtqueue = rx->interthread_queue_ref;
    for(long i = 0; i < 10; i++) {
        sleep(2);
        printf("writer %ld  rx: %p\n", i, rx);
        Evfdq_add(wrtqueue, (void*)i);
//        rtor_interthread_post(wrtr->reactor_ref, postable_function, (void*)i);
    }
}

int test_itq()
{
    ReactorRef react1 = rtor_new();
    EvfdQueueRef q = Evfdq_new();
    QSyncReaderRef rdr = QReader_new(react1,  q, 10);
    QSyncWriterRef wrtr = QWriter_new(react1, q, 10);
    printf("Reactors created %p\n", react1);
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
    UT_ADD(test_itq);
    int rc = UT_RUN();
    return rc;
}
