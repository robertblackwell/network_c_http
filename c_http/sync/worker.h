#ifndef c_http_worker_h
#define c_http_worker_h
//#define _GNU_SOURCE
#include <c_http/common/queue.h>
#include <c_http/sync/sync_handler.h>
#include <pthread.h>
/**
 * @addtogroup group_worker
 */
struct Worker_s;
typedef struct Worker_s Worker, *WorkerRef;

/**
 * Create a new Worker instance. Started by the server in a new thread.
 * Reads tcp socket_fd values from Queue and parses the read data into MessageRef which
 * are passed to the HandlerFuunction to service the request. (see handler.h for signature of SyncHandlerFunction).
 * \param qref
 * \param _id
 * \param handler
 * \return WorkerRef | NULL
 */
WorkerRef Worker_new(QueueRef qref, int _id, SyncHandlerFunction handler);
void Worker_dispose(WorkerRef wref);

/**
 * Actually starts the new thread and calls the private Worker_main() function.
 * \param wref
 * \return
 */
int Worker_start(WorkerRef wref);

pthread_t* Worker_pthread(WorkerRef wref);
/**
 * Performs a pthread_wait() call on the thread associated with this instance of the Worker.
 * \param wref
 */
void Worker_join(WorkerRef wref);

/** @} */

#endif