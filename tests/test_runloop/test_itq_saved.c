
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

typedef struct ITQReader_s {
    EvenfdQueueRef queue;
    int count;
    int expected_count;
} ITQReader, *QSyncReaderRef;


QSyncReaderRef ITQReader_new(EventfdQueueRef queue, int expected_count)
{
    QSyncReaderRef this = malloc(sizeof(ITQReader));
    this->queue = queue;
    this->expected_count = expected_count;
    this->count = 0;
}
void ITQReader_dispose(QSyncReaderRef this)
{
    free(this);
}

typedef struct ITITQWriter_s {
    EvfdQueueRef queue;
    int count_max;
} ITQWriter, *QSyncWriterRef;

QSyncWriterRef ITQWriter_new(EvfdQueueRef queue, int max)
{
    QSyncWriterRef this = malloc(sizeof(ITQReader));
    this->queue = queue;
    this->count_max = max;
}

void ITQWriter_dispose(QSyncReaderRef this)
{
    free(this);
}

void ITQReaderHandler(WQueueRef qw, void* ctx, uint64_t event)
{
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

}

typedef struct WorkerRequest_s {
    ReactorRef response_reactor;
    int id;
} WorkerReqest, *WorkerRequest;

void callback(void* arg)
{
    uintptr_t x = (uintptr_t)arg;
    long lg = (long)x;
    printf("callback id: %ld   thread reactor: %p \n", lg, runloop_get_threads_reactor());
}

void* worker_thread_function(void* arg)
{
    sleep(3);
    ReactorRef rx = (ReactorRef)arg;
    for(int i = 0; i < 10; i++) {
        sleep(2);
        uintptr_t u = (uintptr_t)i;
        void* vstar = (void*)u;
        runloop_interthread_post(rx, callback, vstar);
    }
    sleep(120);
}
static void timer_callback(RtorTimerRef watcher, void* ctx, XrTimerEvent event)
{
    printf("repeating timer \n");
}
void* reader_thread_func(void* arg)
{
    ReactorRef runloop_ref = runloop_new();
    RtorTimerRef tw_2 = runloop_timer_new(runloop_ref, &timer_callback, NULL, 50000, true);

    pthread_t worker;
    int res = pthread_create(&worker, NULL, worker_thread_function, (void*)runloop_ref);

    runloop_run(runloop_ref, 10000);
    printf("reactor has completed\n");
}

int test_itq()
{

    pthread_t rdr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)NULL);

    pthread_join(rdr_thread, NULL);
    return 0;
}

int main()
{
    UT_ADD(test_itq);
    int rc = UT_RUN();
    return rc;
}
