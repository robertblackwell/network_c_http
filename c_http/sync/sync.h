#ifndef c_http_sync_sync.h
#define c_http_sync_sync.h
#include <c_http/common/message.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <c_http//common/queue.h>

typedef struct  sync_server_s sync_server_t, *sync_server_r;
typedef struct sync_worker_s sync_worker_t, *sync_worker_r;
typedef struct sync_connection_s sync_connection_t, *sync_connection_p;

typedef MessageRef(*SyncAppMessageHandler)(MessageRef request_ptr, sync_worker_r worker_ptr);
typedef void(*SyncConnectionMessageHandler)(MessageRef request_ptr, sync_worker_r worker_ptr);

sync_server_r sync_server_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler);
void sync_server_dispose(sync_server_r* srefptr);
void sync_server_listen(sync_server_r server);
void sync_server_terminate(sync_server_r this);

sync_worker_r sync_worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler);
void sync_worker_dispose(sync_worker_r wref);
int sync_worker_start(sync_worker_r wref);
pthread_t sync_worker_pthread(sync_worker_r wref);
int sync_worker_socketfd(sync_worker_r wref);
void sync_worker_join(sync_worker_r wref);


sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_destroy(sync_connection_t* this);
void sync_connection_dispose(sync_connection_t** this_ptr);

/**
 * Read input data until the connection closes
 * Every time a full message is available call the OnMessageHandler with that message.
 * The OnMessageHandler should only return after all processing of that message is completed
 * including output of the reponse.
 * Only returns when the connection errors or is closed by the other end (EOF condition)
 */
int sync_connection_read(sync_connection_t* this);

/**
 *  Writes a complete http message to the associated socket and returns
 *  a status code to indicate success/io error/connection closed by other end
 */
int sync_connection_write(sync_connection_t* this, MessageRef msg_ref);
int sync_connection_sock_fd(sync_connection_t* this);


#endif