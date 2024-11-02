
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

typedef struct QReader_s {
    EventfdQueueRef queue;
    int count;
    int expected_count;
} QReader, *QSyncReaderRef;


QSyncReaderRef QReader_new(EventfdQueueRef queue, int expected_count)
{
    QSyncReaderRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->expected_count = expected_count;
    this->count = 1;
}
void QReader_dispose(QSyncReaderRef this)
{
    free(this);
}

typedef struct QWriter_s {
    EventfdQueueRef queue;
    int count_max;
} QWriter, *QSyncWriterRef;

typedef struct WriterArg {
    int count;
} WriterArg, *WriterArgRef;

QSyncWriterRef QWriter_new(EventfdQueueRef queue, int max)
{
    QSyncWriterRef this = malloc(sizeof(QReader));
    this->queue = queue;
    this->count_max = max;
}

void QWriter_dispose(QSyncReaderRef this)
{
    free(this);
}

void QReaderHandler(RunloopQueueWatcherRef qw, uint64_t event)
{
    void* ctx = qw->queue_event_handler_arg;
    QSyncReaderRef rdr = (QSyncReaderRef)ctx;
    RunloopRef rl = runloop_queue_watcher_get_reactor(qw);
    EventfdQueueRef queue = rdr->queue;
    Functor queue_data = runloop_eventfd_queue_remove(queue);
    PostableFunction pf = queue_data.f;
    void* postable_arg = queue_data.arg;
    runloop_post(rl, pf, postable_arg);

    printf("Q Handler received %p count: %d\n", &queue_data, rdr->count);
    bool x = (long)queue_data.arg == (long)rdr->count;
    assert(x);
    rdr->count++;
    if (rdr->count >= rdr->expected_count) {
        runloop_queue_watcher_deregister(qw);
    }

}

void* reader_thread_func(void* arg)
{
    QSyncReaderRef q_rdr_ctx = (QSyncReaderRef)arg;
    RunloopRef runloop_ref = runloop_new();
    pid_t tid = gettid();
    RunloopQueueWatcherRef qw = runloop_queue_watcher_new(runloop_ref, q_rdr_ctx->queue);
//    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    runloop_queue_watcher_register(qw, QReaderHandler, arg);
    printf("reader thread rl: %p tid: %ld\n", runloop_ref, (long)tid);
    runloop_run(runloop_ref, -1);
    return NULL;
}
/**
 *  The writer thread wants this function run on the reader thread
 */
void writer_post_function(RunloopRef rl, void* arg)
{
    pthread_t mytid = pthread_self();
    pid_t tid = gettid();
    printf("writer post function thread: %ld  rl: %p arg: %p\n", (long)tid, rl, arg);
}
void* writer_thread_func(void* arg)
{
    QSyncWriterRef wrtr = (QSyncWriterRef)arg;
    pid_t tid = gettid();
    printf("writer thread tid: %ld \n", (long)tid);
    for(long i = 1; i <= 10; i++) {
        usleep(500000);
        Functor func = {.f = (void*)&writer_post_function, .arg = (void*) i};
        runloop_eventfd_queue_add(wrtr->queue, func);
    }
    return NULL;
}
/**
 * Test EventFdQueue using a reader and write thread.
 * Writer thread loops a number of times writing data to an instance of EventFdQueue
 * Reader thread has a runloop and a RunloopQueueWatcher waiting for data on the
 * same EventFdQueue.
 * The queue watcher counts the number of times it receives data on the queue
 * and terminates when it has the expected number.
 *
 * IN addition the data from the queue is a postable function and an arg value.
 *
 * The queue watcher function posts that function to the readers runloop.
 * TODO - need a way of counting the number of calls to the writer_post_function.
 * TODO current can only verify writer_post_function is called by debugging
 *
 * test passes if both reader and write counted the same number of time data
 * was transmitted via the queue.
 *
 * Success demonstrate how another thread can post a function to a runloop
 *
 * @return
 */
int test_q()
{
    EventfdQueueRef queue = runloop_eventfd_queue_new();
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
