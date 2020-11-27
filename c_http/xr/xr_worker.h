#ifndef c_http_xr_worker_h
#define c_http_xr_worker_h
//#define _GNU_SOURCE
#include <c_http/xr/types.h>
#include <c_http/xr/evfd_queue.h>
#include <pthread.h>

/**
 * XrWorker - a module that provides an async worker thread that services multiple connections
 * by using the c_http/xr reactor.
 */
struct XrWorker_s;
typedef struct XrWorker_s XrWorker, *XrWorkerRef;

/**
 * Create a new XrWorker instance. Started by the server in a new thread.
 * Reads tcp socket_fd values from Queue and parses the read data into MessageRef which
 * are passed to the HandlerFuunction to service the request. (see handler.h for signature of HandlerFunction).
 * \param qref
 * \param _id
 * \param handler
 * \return XrWorkerRef | NULL
 */
XrWorkerRef XrWorker_new(int _id, XrHandlerFunction handler);
void XrWorker_free(XrWorkerRef wref);

/**
 * Return this workers input queue. A server places new socket fd values
 * into this queue so that the worker can service that fd.
 * \param this
 * \return EvfdQueueRef -
 */
EvfdQueueRef XrWorker_get_queue(XrWorkerRef this);


/**
 * Actually starts the new thread and calls the private XrWorker_main() function.
 * \param wref
 * \return
 */
int XrWorker_start(XrWorkerRef wref);

pthread_t* XrWorker_pthread(XrWorkerRef wref);
/**
 * Performs a pthread_wait() call on the thread associated with this instance of the XrWorker.
 * \param wref
 */
void XrWorker_join(XrWorkerRef wref);
#endif