#ifndef c_http_sync_sync_connection_h
#define c_http_sync_sync_connection_h
#include <http_in_c/http/message.h>
#include <http_in_c/http/parser.h>
#include <http_in_c/saved/sync_handler.h>
#define sync_connection_TAG "SYNCCONN"
#include <http_in_c/check_tag.h>

typedef struct sync_connection_s sync_connection_t, *sync_connection_p;



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