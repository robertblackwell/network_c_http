
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

#include "runloop/kqueue_runloop/rl_events_internal.h"
// #include <src/runloop/rl_internal.h>
uint64_t local_gettid() {
    #ifdef LINUX_FLAG
        return (uint64_t)gettid();
    #elif defined(APPLE_FLAG)
        uint64_t tid;
        pthread_threadid_np(NULL, &tid);
        return tid;
    #endif
    return tid;
}

#define QRCTX_Tag "QRCTX"
typedef struct QReader_s {
    RBL_DECLARE_TAG;
    RunloopRef        rdr_runloop_ref;
    UserEventQueueRef ue_queue;
    int count;
    int expected_count;
    RBL_DECLARE_END_TAG;
} QReader, *QReaderRef;


QReaderRef queue_reader_new(RunloopRef rl, UserEventQueueRef ue_queue, int expected_count)
{
    QReaderRef this = malloc(sizeof(QReader));
    RBL_SET_TAG(QRCTX_Tag, this);
    RBL_SET_END_TAG(QRCTX_Tag, this);
    this->rdr_runloop_ref = rl;
    this->ue_queue = ue_queue;
    this->expected_count = expected_count;
    this->count = 0;
    return this;
}
void queue_reader_free(QReaderRef this)
{
    RBL_CHECK_TAG(QRCTX_Tag, this);
    RBL_CHECK_END_TAG(QRCTX_Tag, this);
    free(this);
}

#define QWCTX_Tag "QWCTX"
typedef struct QWriter_s {
    RBL_DECLARE_TAG;
    RunloopRef      rdr_runloop_ref;
    UserEventQueueRef ue_queue;
    int count_max;
    long post_count;
    RBL_DECLARE_END_TAG;
} QWriter, *QWriterRef;

typedef struct WriterArg {
    long count;
    long post_count;
    QWriterRef qwriter_ref;
} WriterArg, *WriterArgRef;

QWriterRef queue_writer_new(RunloopRef rl,  UserEventQueueRef ue_queue, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    RBL_SET_TAG(QWCTX_Tag, this);
    RBL_SET_END_TAG(QWCTX_Tag, this);
    this->rdr_runloop_ref = rl;
    this->ue_queue = ue_queue;
    this->count_max = max;
    this->post_count = 0;
    return this;
}
void queue_writer_free(QReaderRef this)
{
    RBL_CHECK_TAG(QWCTX_Tag, this);
    RBL_CHECK_END_TAG(QWCTX_Tag, this);
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
/**
 * Called whenever the user-event associated with the queue is triggered by the
 * queue writer
 */
void queue_cb(RunloopRef rl, void* q_rdr_ctx_arg)
{
    QReaderRef rdr = (QReaderRef)q_rdr_ctx_arg;
    RBL_CHECK_TAG(QRCTX_Tag, rdr)
    RBL_CHECK_END_TAG(QRCTX_Tag, rdr)
    UserEventQueueRef ue_queue = rdr->ue_queue;
    user_event_queue_verify(ue_queue);
    runloop_user_event_verify(ue_queue->user_event);
    while (1) {
        Functor queue_data = user_event_queue_remove(ue_queue);
        if (queue_data.f == NULL) {
            printf("queue_cb - queue is empty \n");
            return;
       }
        rdr->count++;
        printf("Q callback received %p count: %d\n", &queue_data, rdr->count);
        PostableFunction pf = queue_data.f;
        void* postable_arg = queue_data.arg;
        runloop_post(rl, pf, postable_arg);
    }
#ifdef APPLE_FLAG
    user_event_queue_register(ue_queue, queue_cb, q_rdr_ctx_arg);
#endif
}
/**
 * This thread creates a queue watcher that will get an event from its runloop whenever
 * the user_event associated with the queue is triggered by the writer thread.
 * This will be notified by a call to the queue_postable() function. This function
 * will be called from the readers runloop and will read data from the queue
 */
void* reader_thread_func(void* arg)
{
    QReaderRef rctx = (QReaderRef)arg;
    RunloopRef runloop_ref = rctx->rdr_runloop_ref;
    uint64_t tid = local_gettid();
    UserEventQueueRef ue_queue = rctx->ue_queue;
    user_event_queue_register(ue_queue, queue_cb, arg);
    printf("reader thread rl: %p tid: %llu\n", runloop_ref, tid);
    runloop_run(runloop_ref, 5000);
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
    uint64_t tid = local_gettid();
    printf("writer post function thread: %llu  rl: %p arg: %p arg->count: %ld post_count: %ld\n",
           tid, rl, arg, count, pcount);
}
/**
 *  arg is a QWriterRef
 */
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    uint64_t tid = local_gettid();
    printf("writer thread tid: %llu \n", tid);
    for(long i = 0; i < wrtr->count_max; i++) {
        // usleep(5000);
        WriterArgRef writer_arg_ref = writer_arg_new(wrtr, i);
        Functor func = {.f = (void*)&writer_post_function, .arg = (void*) writer_arg_ref};
        user_event_queue_add(wrtr->ue_queue, func);
        printf("writer loop i: %ld  count_max: %ld post_count: %ld\n", i, (long)wrtr->count_max, wrtr->post_count);
    }
    sleep(2);
    return NULL;
}
/**
 * Test EventFdQueue using a reader and write thread.
 * Writer thread loops a number of times writing data to an instance of UserEventQueue
 * Reader thread has a runloop and a RunloopQueueWatcher waiting for data on the
 * same UserEventQueue.
 * The queue watcher counts the number of times it receives data on the queue
 * and terminates when it has the expected number.
 *
 * In addition the data from the queue is a postable function and an arg value.
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
    int nbr_writers = 5;
    int nbr_readers = 1;
    QReaderRef rdr[nbr_readers];
    pthread_t  reader_threads[nbr_readers];
    QWriterRef writers[nbr_writers];
    pthread_t writer_threads[nbr_writers];

    RunloopRef rdr_runloop_ref = runloop_new();
    UserEventQueueRef queue = user_event_queue_new(rdr_runloop_ref);
    for (int ir = 0; ir < nbr_readers; ++ir) {
        rdr[ir] = queue_reader_new(rdr_runloop_ref, queue, 10);
        int r_rdr = pthread_create(&(reader_threads[ir]), NULL, reader_thread_func, (void*)rdr[ir]);
    }
    sleep(2);
    for (int iw=0; iw < nbr_writers; ++iw) {
        writers[iw] = queue_writer_new(rdr_runloop_ref, queue, 6);
        int w = pthread_create(&(writer_threads[iw]), NULL, writer_thread_func, (void*)writers[iw]);
    }

    long rtotal = 0;
    for (int ir = 0; ir < nbr_readers; ++ir) {
        pthread_join(reader_threads[ir], NULL);
        rtotal += rdr[ir]->count;
    }
    long wtotal = 0;
    long ptotal = 0;
    for (int iw=0; iw < nbr_writers; ++iw) {
        pthread_join(writer_threads[iw], NULL);
        wtotal += writers[iw]->count_max;
        ptotal += writers[iw]->post_count;
    }
    printf("rtotal: %ld wtotal: %ld ptotal: %ld\n", rtotal, wtotal, ptotal);
    UT_TRUE((wtotal == rtotal));
    UT_TRUE((wtotal == ptotal));
    return 0;
}

int main()
{
    UT_ADD(test_q);
    int rc = UT_RUN();
    return rc;
}
