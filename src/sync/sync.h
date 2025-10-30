#ifndef c_http_sync_sync.h
#define c_http_sync_sync.h
#include <src/http/http_message.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <src/common/queue.h>

typedef struct  sync_server_s sync_server_t, *sync_server_r;
typedef struct sync_worker_s sync_worker_t, *sync_worker_r;
typedef struct sync_connection_s sync_connection_t, *sync_connection_p;
typedef struct sync_client_s sync_client_t, *sync_client_r;

typedef HttpMessageRef(*SyncAppMessageHandler)(HttpMessageRef request_ptr, sync_worker_t* worker_ptr);

// The 2 handlers should return one of:
// HPE_OK       - the connection will continue to read and parse data
// HPE_PAUSED   - the connection will cease reading and processing data and will pause the parser
//                do this only in a client when you can be sure the server will have only sent
//                a single response message.
//                remember to 'resume' the underlying llhttp parser
// -1             Only if your code has detected an error in the message
//
typedef int(*SyncConnectionServerMessageHandler)(HttpMessageRef request_ptr, sync_worker_t* worker_ptr);
typedef int(*SyncConnectionClientMessageHandler)(HttpMessageRef request_ptr, sync_client_t* client_ptr);

sync_server_r sync_server_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler);
void sync_server_dispose(sync_server_r* srefptr);
void sync_server_listen(sync_server_r server);
void sync_server_terminate(sync_server_r this);

/**
 * SYNC_WORKER_QUEUE
 *
 * there are 2 ways that the server and worker can interact in regard to listening for incoming connections.
 * -    the first method SYNC_WORKER_QUEUE is defined. The main thread does the listening and when a connection is made
 *      it passes the socket to a worker thread via a synchronized queue
 *
 * -    the second method SYNC_WORKER_QUEUE NOT defined - the main thread creates the listener socket but each worker
 *      issues its own accept() call - the Linux OS ensures that only one thread waiting for an incoming
 *      connection will be worken up to handle the connection. The main thread does nothing after starting the worker
 *      threads. It joust issues pthread_join() calls to wait for all the workers to terminate.
 */
#ifdef SYNC_WORKER_QUEUE
sync_worker_r sync_worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler);
#else
sync_worker_r sync_worker_new(int listen_socket, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler);
#endif
void sync_worker_dispose(sync_worker_r wref);
int sync_worker_start(sync_worker_r wref);
pthread_t sync_worker_pthread(sync_worker_r wref);
int sync_worker_socketfd(sync_worker_r wref);
void sync_worker_join(sync_worker_r wref);

sync_client_t* sync_client_new(size_t read_buffer_size);
void sync_client_init(sync_client_t* this, size_t read_buffer_size);
void sync_client_free(sync_client_t* this);
void sync_client_connect(sync_client_t* this, char* host, int port);
//void sync_client_request_round_trip(sync_client_t* this, HttpMessageRef request, SyncConnectionClientMessageHandler handler);
void* sync_client_get_userptr(sync_client_t* this);
void sync_client_set_userptr(sync_client_t* this, void* userptr);
void sync_client_close(sync_client_t* this);

sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size); //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size); //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_destroy(sync_connection_t* this);
void sync_connection_dispose(sync_connection_t** this_ptr);
/**
 * Gets a message into msg_ptr and returns 1, or
 * Puts NULL in msg_ptr and returns 0 - io error or parse error - you should close the connection
 */
int sync_connection_read_message(sync_connection_t* this, HttpMessageRef *msg_ptr);
// To be used by sync_server/sync_worker.
// These next 2 functions will return when is=t has consumed all input data or hit a parse error
// Not when a message is complete.
// When a message is complete the handler will be called
//int sync_connection_read_request(sync_connection_t* this, SyncConnectionServerMessageHandler handler, sync_worker_t* worker_ptr);
// to be used by sync_client
//int sync_connection_read_response(sync_connection_t* this, SyncConnectionClientMessageHandler handler, sync_client_t* client_ptr);
void sync_connection_close(sync_connection_t* this);
/**
 *  Writes a complete http message to the associated socket and returns
 *  a status code to indicate success/io error/connection closed by other end
 */
int sync_connection_write(sync_connection_t* this, HttpMessageRef msg_ref);
int sync_connection_sock_fd(sync_connection_t* this);


#endif