#ifndef c_http_sync_sync_internal_h
#define c_http_sync_sync_internal_h
#include <http_in_c/logger.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/common/queue.h>
#include <http_in_c/http/parser.h>

#include <http_in_c/sync/sync.h>
#include <http_in_c/sync/tags.h>


#define MAX_THREADS 100
struct sync_server_s {
    DECLARE_TAG;
    int                         port;
    size_t                      read_buffer_size;
    socket_handle_t             socket_fd;
    int                         nbr_workers;
    SyncAppMessageHandler       app_handler;
    QueueRef                    qref;
    sync_worker_r               worker_tab[MAX_THREADS];
};

struct sync_worker_s {
    DECLARE_TAG;
    bool                active;
    int                 active_socket;
    sync_connection_t*  connection_ptr;
#ifdef SYNC_WORKER_QUEUE
    QueueRef            qref;
#else
    int                 listen_socket;
#endif
    pthread_t           pthread;
    int                 id;
    size_t              read_buffer_size;
    SyncAppMessageHandler app_handler;
};


struct sync_connection_s
{
    DECLARE_TAG;
    http_parser_t*               m_parser;
    IOBufferRef                  m_iobuffer;
    int                          socketfd;
    size_t                       read_buffer_size;
    bool is_server_callback;  // boolean tag to discriminate the callback union
    union callback {
        struct {
            SyncConnectionServerMessageHandler server_handler;
            sync_worker_t* worker_ptr; // a connection being used by a sync_server/sync_worker
        } server_cb;
        struct {
            SyncConnectionClientMessageHandler client_handler;
            sync_client_t* client_ptr; // a connection being used by a sync_client
        } client_cb;
    } callback;
};

struct sync_client_s {
    DECLARE_TAG;
    void*   user_ptr;
    size_t read_buffer_size;
    sync_connection_t* connection_ptr;
    pthread_mutex_t mutex;
};


#endif