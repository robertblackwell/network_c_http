#ifndef c_ceg_worker_h
#define c_ceg_worker_h
//#define _GNU_SOURCE
#include <c_eg/queue.h>
#include <c_eg/handler.h>
#include <pthread.h>

struct Worker_s;
typedef struct Worker_s Worker;

/**
 * Create a new Worker instance. Started by the server in a new thread.
 * Reads tcp socket_fd values from Queue and parses the read data into Message* which
 * are passed to the HandlerFuunction to service the request. (see handler.h for signature of HandlerFunction).
 * \param qref
 * \param _id
 * \param handler
 * \return Worker* | NULL
 */
Worker* Worker_new(Queue* qref, int _id, HandlerFunction handler);
void Worker_free(Worker* wref);

/**
 * Actually starts the new thread and calls the private Worker_main() function.
 * \param wref
 * \return
 */
int Worker_start(Worker* wref);

pthread_t* Worker_pthread(Worker* wref);
/**
 * Performs a pthread_wait() call on the thread associated with this instance of the Worker.
 * \param wref
 */
void Worker_join(Worker* wref);
#endif