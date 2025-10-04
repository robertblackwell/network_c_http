
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
/* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>

#include <sys/epoll.h>
#include <rbl/unittest.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
#include <src/runloop/rl_internal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InterthreadQueue
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void itqueue_event_handler(RunloopQueueWatcherRef qwref, uint64_t events)
//{
//    void* ctx = qwref->queue_postable_arg;
//    RunloopRef rl = runloop_queue_watcher_get_reactor(qwref);
//    EventfdQueueRef queue = qwref->queue;
//    Functor queue_data = runloop_eventfd_queue_remove(queue);
//    RBL_ASSERT((!Functor_is_empty(&(queue_data))), "An empty entry in a func list");
//    PostableFunction pf = queue_data.f;
//    void* postable_arg = queue_data.arg;
//    runloop_post(rl, pf, postable_arg);
//}
//
//#define InterThreadQ_TYPE "itQu"
//typedef struct InterthreadQueue {
//    RBL_DECLARE_TAG;
//    EventfdQueueRef queue;
//    RunloopRef runloop;
//    RunloopQueueWatcherRef qwatcher_ref;
//    RBL_DECLARE_END_TAG;
//} InterthreadQueue_s, InterthreadQueue, *InterthreadQueueRef;
//
//InterthreadQueueRef itqueue_new(RunloopRef rl)
//{
//    InterthreadQueueRef itq_ref = malloc(sizeof(InterthreadQueue));
//    RBL_SET_TAG(InterThreadQ_TYPE, itq_ref);
//    RBL_SET_END_TAG(InterThreadQ_TYPE, itq_ref)
//    itq_ref->runloop = rl;
//    itq_ref->queue = runloop_eventfd_queue_new();
//    itq_ref->qwatcher_ref = runloop_queue_watcher_new(itq_ref->runloop, itq_ref->queue);
//    runloop_queue_watcher_register(itq_ref->qwatcher_ref, &itqueue_event_handler, itq_ref);
//    return itq_ref;
//}
//void itqueue_add(InterthreadQueueRef qref, Functor func)
//{
//    runloop_eventfd_queue_add(qref->queue, func);
//}
//Functor itqueue_remove(InterthreadQueueRef qref)
//{
//    Functor func = runloop_eventfd_queue_remove(qref->queue);
//    return func;
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reader cntext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define Reader_TAG "Qreader"
typedef struct QReader_s {
    RBL_DECLARE_TAG;
    InterthreadQueueRef queue;
    RunloopRef runloop_ref;
    int count;
    int expected_count;
    RBL_DECLARE_END_TAG;
} QReader, *QReaderRef;


QReaderRef queue_reader_new(RunloopRef rl, InterthreadQueueRef queue, int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    RBL_SET_TAG(Reactor_TAG, this);
    RBL_SET_END_TAG(Reactor_TAG, this);
    this->queue = queue;
    this->runloop_ref = rl;
    this->expected_count = expected_count;
    this->count = 1;
    return this;
}
void queue_reader_free(QReaderRef this)
{
    free(this);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Writer context
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define Writer_TAG "QWrtr"
#define WrtrArg_TAG "WrtrArg"
typedef struct QWriter_s {
    RBL_DECLARE_TAG;
    InterthreadQueueRef queue;
    RunloopRef rdr_runloop_ref;
    int count_max;
    long post_count;
    RBL_DECLARE_END_TAG;
} QWriter, *QWriterRef;

typedef struct WriterArg {
    RBL_DECLARE_TAG;
    long count;
    long post_count;
    QWriterRef qwriter_ref;
    RBL_DECLARE_END_TAG;
} WriterArg, *WriterArgRef;

QWriterRef queue_writer_new(RunloopRef rl, InterthreadQueueRef queue, int max)
{
    QWriterRef this = malloc(sizeof(QReader));
    RBL_SET_TAG(Writer_TAG, this);
    RBL_SET_END_TAG(Writer_TAG, this)
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
    RBL_SET_TAG(WrtrArg_TAG, waref)
    RBL_SET_END_TAG(WrtrArg_TAG, waref)
    waref->count = count;
    waref->post_count = 1;
    waref->qwriter_ref = qwrtr_ref;
    return waref;
}
//void queue_postable(RunloopQueueWatcherRef qw, uint64_t event)
//{
//    void* ctx = qw->queue_postable_arg;
//    QReaderRef rdr = (QReaderRef)ctx;
//    RunloopRef rl = runloop_queue_watcher_get_reactor(qw);
//    InterthreadQueueRef queue = rdr->queue;
//    Functor queue_data = runloop_eventfd_queue_remove(queue->queue);
//
//    WriterArgRef writer_arg_ref = (WriterArgRef)queue_data.arg;
//
//    printf("Q Handler received %p count: %d\n", &queue_data, rdr->count);
//    bool x = (long)writer_arg_ref->count == (long)rdr->count;
//    RBL_ASSERT(x, "count check failed in queue_postable");
//    rdr->count++;
//    // now call the postable function passed by the writer
//
//    PostableFunction pf = queue_data.f;
//    void* postable_arg = queue_data.arg;
//    runloop_post(rl, pf, postable_arg);
//
//    if (rdr->count >= rdr->expected_count) {
//        runloop_queue_watcher_deregister(qw);
//    }
//
//}
/**
 * arg is QReaderRef
 */
void* reader_thread_func(void* arg)
{
    QReaderRef q_rdr_ctx = (QReaderRef)arg;
    RunloopRef runloop_ref = q_rdr_ctx->runloop_ref;
    pid_t tid = gettid();
//    RunloopQueueWatcherRef qw = runloop_queue_watcher_new(runloop_ref, q_rdr_ctx->queue);
//    runloop_queue_watcher_register(qw, queue_postable, arg);
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
    if(count == qwrtr_ref->count_max) {
        runloop_queue_watcher_deregister(qwrtr_ref->queue->qwatcher_ref);
//        close(qwrtr_ref->rdr_runloop_ref->epoll_fd);
    }
}
/**
 *  arg is a QWriterRef
 */
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    InterthreadQueueRef itqref = wrtr->queue;
    pid_t tid = gettid();
    printf("writer thread tid: %ld \n", (long)tid);
    for(long i = 1; i <= 10; i++) {
        usleep(500000);
        WriterArgRef writer_arg_ref = writer_arg_new(wrtr, i);
        Functor func = {.f = (void*)&writer_post_function, .arg = (void*) writer_arg_ref};
        itqueue_add(itqref, func);
//        runloop_eventfd_queue_add(wrtr->queue, func);
//        if(wrtr->rdr_runloop_ref->interthread_queue_ref != NULL) {
//            runloop_eventfd_queue_add(wrtr->rdr_runloop_ref->interthread_queue_ref, func);
//        }
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
    InterthreadQueueRef itqref = itqueue_new(rdr_runloop_ref);
    QReaderRef rdr = queue_reader_new(rdr_runloop_ref, itqref, 10);
    QWriterRef wrtr = queue_writer_new(rdr_runloop_ref, itqref, 10);

    pthread_t rdr_thread;
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    printf("expected_count : %ld count: %ld count_max: %ld post_count: %ld",(long)rdr->expected_count, (long)rdr->count, (long)wrtr->count_max, (long)wrtr->post_count);
    UT_TRUE((rdr->expected_count == wrtr->post_count));
    UT_TRUE((wrtr->post_count == wrtr->count_max));
    return 0;
}

int main()
{
    UT_ADD(test_q);
    int rc = UT_RUN();
    return rc;
}
