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

typedef struct  SyncServer_s Server, *SyncServerRef;
typedef struct Worker_s Worker, *WorkerRef;
typedef struct sync_connection_s sync_connection_t, *sync_connection_p;

typedef MessageRef(*SyncAppMessageHandler)(MessageRef request_ptr, WorkerRef worker_ptr);
typedef void(*SyncConnectionMessageHandler)(MessageRef request_ptr, WorkerRef worker_ptr);

SyncServerRef SyncServer_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler);
void SyncServer_dispose(SyncServerRef* srefptr);
void SyncServer_listen(SyncServerRef server);
void SyncServer_terminate(SyncServerRef this);

WorkerRef Worker_new(QueueRef qref, int ident, size_t read_buffer_size, SyncAppMessageHandler app_handler);
void Worker_dispose(WorkerRef wref);
int Worker_start(WorkerRef wref);
pthread_t Worker_pthread(WorkerRef wref);
int Worker_socketfd(WorkerRef wref);
void Worker_join(WorkerRef wref);


sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, void* handler_context);
void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size, SyncConnectionMessageHandler handler, void* handler_context);
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