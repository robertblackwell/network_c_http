
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>

#include <rbl/unittest.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
// #include <src/runloop/rl_internal.h>

typedef struct QReader_s {
    RunloopRef      rdr_runloop_ref;
    UserEventQueueRef queue;
    RunloopQueueWatcherRef queue_watcher;
    int count;
    int expected_count;
} QReader, *QReaderRef;


QReaderRef queue_reader_new(RunloopRef rl, UserEventQueueRef queue, RunloopQueueWatcherRef qw, int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    this->rdr_runloop_ref = rl;
    this->queue = queue;
    this->queue_watcher = qw;
    this->expected_count = expected_count;
    this->count = 1;
    return this;
}
void queue_reader_free(QReaderRef this)
{
    free(this);
}

typedef struct QWriter_s {
    RunloopRef      rdr_runloop_ref;
    UserEventQueueRef queue;
    int count_max;
    long post_count;
} QWriter, *QWriterRef;

typedef struct WriterArg {
    long count;
    long post_count;
    QWriterRef qwriter_ref;
} WriterArg, *WriterArgRef;

QWriterRef queue_writer_new(RunloopRef rl,  UserEventQueueRef queue, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    this->rdr_runloop_ref = rl;
    this->queue = queue;
    this->count_max = max;
    this->post_count = 0;
    return this;
}
void queue_writer_free(QReaderRef this)
{
    free(this);
}
WriterArgRef writer_arg_new(QWriterRef qwrtr_ref, long count)
{
    WriterArgRef waref = malloc(sizeof(WriterArg));
    waref->count = count;
    waref->post_count = 1;
    waref->qwriter_ref = qwrtr_ref;
    return waref;
}
void queue_postable(RunloopRef rl, void* q_rdr_ctx_arg)
{
    QReaderRef rdr = (QReaderRef)q_rdr_ctx_arg;
    UserEventQueueRef queue = rdr->queue;
    RunloopQueueWatcherRef qw = rdr->queue_watcher;
    Functor queue_data = runloop_user_event_queue_remove(queue);

    WriterArgRef writer_arg_ref = (WriterArgRef)queue_data.arg;

    printf("Q Handler received %p count: %d\n", &queue_data, rdr->count);
    bool x = (long)writer_arg_ref->count == (long)rdr->count;
    RBL_ASSERT(x, "count check failed in queue_postable");
    rdr->count++;
    // now call the postable function passed by the writer

    PostableFunction pf = queue_data.f;
    void* postable_arg = queue_data.arg;
    runloop_post(rl, pf, postable_arg);

    if (rdr->count >= rdr->expected_count) {
//        RunloopRef rl = rdr->rdr_runloop_ref;
        runloop_queue_watcher_deregister(qw);
        runloop_queue_watcher_free(qw);
        runloop_close(rl);
    }

}
/**
 * arg is QReaderRef
 */
void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    RunloopRef runloop_ref = q_rdr_ctx->rdr_runloop_ref;
    pid_t tid = gettid();
    RunloopQueueWatcherRef qw = runloop_queue_watcher_new(runloop_ref, q_rdr_ctx->queue);
//    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    runloop_queue_watcher_register(qw, queue_postable, arg);
    printf("reader thread rl: %p tid: %ld\n", runloop_ref, (long)tid);
    runloop_run(runloop_ref, -1);
    return NULL;
}
/**
 *  The writer thread wants this function run on the reader thread
 */
void writer_post_function(RunloopRef rl, void* arg)
{
    WriterArgRef wref = (WriterArgRef)arg;
    QWriterRef qwrtr_ref = wref->qwriter_ref;
    qwrtr_ref->post_count++;
    long pcount = qwrtr_ref->post_count;
    long count = wref->count;
    pthread_t mytid = pthread_self();
    pid_t tid = gettid();
    printf("writer post function thread: %ld  rl: %p arg: %p arg->count: %ld post_count: %ld\n",
           (long)tid, rl, arg, count, pcount);
}
/**
 *  arg is a QWriterRef
 */
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    pid_t tid = gettid();
    printf("writer thread tid: %ld \n", (long)tid);
    for(long i = 1; i <= 10; i++) {
        usleep(500000);
        WriterArgRef writer_arg_ref = writer_arg_new(wrtr, i);
        Functor func = {.f = (void*)&writer_post_function, .arg = (void*) writer_arg_ref};
        runloop_user_event_queue_add(wrtr->queue, func);
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
    RunloopRef rdr_runloop_ref = runloop_new();
    UserEventQueueRef queue = runloop_user_event_queue_new(rdr_runloop_ref);
    RunloopQueueWatcherRef qw = runloop_queue_watcher_new(rdr_runloop_ref, queue);
    QReaderRef rdr = queue_reader_new(rdr_runloop_ref, queue, qw, 10);
    QWriterRef wrtr = queue_writer_new(rdr_runloop_ref, queue, 10);

    pthread_t rdr_thread;
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    UT_TRUE((rdr->expected_count == rdr->count));
    UT_TRUE((rdr->count == wrtr->count_max));
    UT_TRUE((wrtr->post_count+1 == wrtr->count_max));
    return 0;
}

int main()
{
    UT_ADD(test_q);
    int rc = UT_RUN();
    return rc;
}
