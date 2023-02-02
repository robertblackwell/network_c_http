#ifndef c_http_sync_worker_h
#define c_http_sync_worker_h
//#define _GNU_SOURCE
#include <c_http/common/queue.h>
#include <c_http/sync/sync_handler.h>
#include <pthread.h>
struct Worker_s;
typedef struct Worker_s Worker, *WorkerRef;

WorkerRef Worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler);
void Worker_dispose(WorkerRef wref);
int Worker_start(WorkerRef wref);
pthread_t* Worker_pthread(WorkerRef wref);
void Worker_join(WorkerRef wref);

#endif