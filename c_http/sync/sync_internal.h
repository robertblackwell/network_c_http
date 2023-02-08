#ifndef c_http_sync_sync_internal_h
#define c_http_sync_sync_internal_h
#include <c_http/logger.h>
#include <c_http/socket_functions.h>
#include <c_http/common/queue.h>
#include <c_http/http_parser/ll_parser.h>

#include <c_http/sync/sync.h>
#include <c_http/sync/tags.h>


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
    QueueRef            qref;
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