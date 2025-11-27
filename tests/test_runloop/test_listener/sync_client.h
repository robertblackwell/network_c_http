#ifndef c_http_api_sync_client_h
#define c_http_api_sync_client_h
#include <common/list.h>
#include <src/http/http_message.h>
#include <src/http/http_message_parser.h>
#include <stdio.h>
#include <rbl/check_tag.h>
#define SYNC_SERVER_TAG "SYNCSVER"
#define SYNC_CONNECTION_TAG "SYNCCONN"
#define SYNC_WORKER_TAG "SYWRKR"

typedef struct sync_client_s sync_client_t, *sync_client_r;
typedef struct sync_connection_s sync_connection_t, *sync_connection_p;
struct sync_connection_s
{
    RBL_DECLARE_TAG;
    HttpMessageParser*           m_parser;
    IOBufferRef                  m_iobuffer;
    int                          socketfd;
    size_t                       read_buffer_size;
    ListRef                      input_list;
    int                          reader_status;     // 1 indicates message received 0 otherwise

};

struct sync_client_s {
    RBL_DECLARE_TAG;
    void*   user_ptr;
    size_t read_buffer_size;
    // sync_connection_t* connection_ptr;
    pthread_mutex_t mutex;
    int socket_fd;
};

sync_client_t* sync_client_new(size_t read_buffer_size);
void sync_client_init(sync_client_t* this, size_t read_buffer_size);
void sync_client_free(sync_client_t* this);
void sync_client_connect(sync_client_t* this, char* host, int port);
void* sync_client_get_userptr(sync_client_t* this);
void sync_client_set_userptr(sync_client_t* this, void* userptr);
void sync_client_close(sync_client_t* this);

sync_connection_t* sync_connection_new(int socketfd, size_t read_buffer_size); //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_init(sync_connection_t* this, int socketfd, size_t read_buffer_size); //, SyncConnectionServerMessageHandler handler, sync_worker_r worker_ref);
void sync_connection_destroy(sync_connection_t* this);
void sync_connection_dispose(sync_connection_t** this_ptr);
// int sync_connection_read_message(sync_connection_t* this, HttpMessageRef *msg_ptr);
void sync_connection_close(sync_connection_t* this);
// int sync_connection_write(sync_connection_t* this, HttpMessageRef msg_ref);
// int sync_connection_sock_fd(sync_connection_t* this);


#endif