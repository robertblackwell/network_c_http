
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

/**
 * Test UserEventQueue using one (and only one) reader threads and multiple writer thread.
 *
 * All threads share a single UserEventQueue.
 *
 * Writer thread loops a number of times writing a Functor (callback_function, void* arg) to the instance of
 * UserEventQueue using the user_event_queue_add(queue, functor)
 *
 * Reader thread has a runloop and sets a callback (queue_cb()) to be called whenever the UserEvent property in the UserEventQueue
 * is fired/triggered.
 *
 * The callback (queue_cb()) removes all Functor entries on the queue and dispatches them, using runloop_post(),
 * one by one.
 *
 * The test:
 * The writer threads each count:
 * -    the number of Functors they add to the queue and store this in wrtr->count_max
 * -    the number of times the writer_postable() is called wrtr->post_count.
 * -    when all writer threads complete these per-thread values are summed to get wtotal and ptotal
 *
 *  The reader thread(s) counts the number of times that queue_cb() is called rdr->count. These per-thread values
 *  are summed over all reader threads to get a value rtotal.
 *
 *  The test is assert((rtotal == ptotal) && (ptotal == wtotal))
 *
 */

// this type def should be such that the result value can be printf'd with %ul specification
typedef unsigned long LocalTid;
LocalTid local_gettid() {
    uint64_t tid;
    #ifdef LINUX_FLAG
        tid = (uint64_t)gettid();
    #elif defined(APPLE_FLAG)
        pthread_threadid_np(NULL, &tid);
        return (LocalTid)tid;
    #endif
    return (LocalTid)tid;
}

#define QRCTX_Tag "QRCTX"
typedef struct QReader_s {
    RBL_DECLARE_TAG;
    RunloopRef        rdr_runloop_ref;
    UserEventQueueRef ue_queue;
    int count;
    int expected_count;
    LocalTid tid;
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
    LocalTid rdr_thread_id;
    int count_max;
    long post_count;
    RBL_DECLARE_END_TAG;
} QWriter, *QWriterRef;

typedef struct WriterArg {
    long count;
    long post_count;
    QWriterRef qwriter_ref;
} WriterArg, *WriterArgRef;

QWriterRef queue_writer_new(RunloopRef rl,  UserEventQueueRef ue_queue, LocalTid rdr_thread_id, int max)
{
    QWriterRef this = malloc(sizeof(QWriter));
    RBL_SET_TAG(QWCTX_Tag, this);
    RBL_SET_END_TAG(QWCTX_Tag, this);
    this->rdr_runloop_ref = rl;
    this->ue_queue = ue_queue;
    this->rdr_thread_id = rdr_thread_id;
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
void* reader_thread_func(void* arg)
{
    QReaderRef rctx = (QReaderRef)arg;
    RunloopRef runloop_ref = rctx->rdr_runloop_ref;
    LocalTid tid = local_gettid();
    rctx->tid = tid;
    UserEventQueueRef ue_queue = rctx->ue_queue;
    user_event_queue_arm(ue_queue);
    printf("reader thread rl: %p tid: %lu\n", runloop_ref, tid);
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
    LocalTid tid = local_gettid();
    uint64_t tid2 = wref->qwriter_ref->rdr_thread_id;
    assert(tid == tid2); // this tests that the post function is running on the thread with the runloop
    printf("writer post function thread: %lu rdr_thread_id: %lu rl: %p arg: %p arg->count: %ld post_count: %ld\n",
           (unsigned long)tid, (unsigned long)wref->qwriter_ref->rdr_thread_id, rl, arg, count, pcount);
}
/**
 *  arg is a QWriterRef
 */
void* writer_thread_func(void* arg)
{
    QWriterRef wrtr = (QWriterRef)arg;
    LocalTid tid = local_gettid();
    printf("writer thread tid: %lu \n", (unsigned long)tid);
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
int test_q()
{
    int nbr_writers = 5;
    int nbr_readers = 1; // dont change this value
    QReaderRef rdr[nbr_readers];
    pthread_t  reader_threads[nbr_readers];
    QWriterRef writers[nbr_writers];
    pthread_t writer_threads[nbr_writers];

    RunloopRef rdr_runloop_ref = runloop_new();
    UserEventQueueRef queue = user_event_queue_new(rdr_runloop_ref, 100);
    for (int ir = 0; ir < nbr_readers; ++ir) {
        rdr[ir] = queue_reader_new(rdr_runloop_ref, queue, 10);
        int r_rdr = pthread_create(&(reader_threads[ir]), NULL, reader_thread_func, (void*)rdr[ir]);
    }

    sleep(2);
    LocalTid reader_tid = rdr[0]->tid;
    for (int iw=0; iw < nbr_writers; ++iw) {
        writers[iw] = queue_writer_new(rdr_runloop_ref, queue, reader_tid, 6);
        int w = pthread_create(&(writer_threads[iw]), NULL, writer_thread_func, (void*)writers[iw]);
    }

    long wtotal = 0;
    long ptotal = 0;
    for (int iw=0; iw < nbr_writers; ++iw) {
        pthread_join(writer_threads[iw], NULL);
        wtotal += writers[iw]->count_max;
        ptotal += writers[iw]->post_count;
    }
    printf("wtotal: %ld ptotal: %ld\n", wtotal, ptotal);
    UT_TRUE((wtotal == ptotal));
    return 0;
}

int main()
{
    UT_ADD(test_q);
    int rc = UT_RUN();
    return rc;
}
