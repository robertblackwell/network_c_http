#ifndef c_http_sync_sync_internal_h
#define c_http_sync_sync_internal_h
#include <rbl/logger.h>
#include <src/common/socket_functions.h>
#include <src/common/queue.h>
#include <src/http_protocol/http_message_parser.h>

#include <src/sync/sync.h>
#include <src/sync/tags.h>


#define MAX_THREADS 100
struct sync_server_s {
    RBL_DECLARE_TAG;
    int                         port;
    size_t                      read_buffer_size;
    socket_handle_t             socket_fd;
    int                         nbr_workers;
    SyncAppMessageHandler       app_handler;
    QueueRef                    qref;
    sync_worker_r               worker_tab[MAX_THREADS];
};

struct sync_worker_s {
    RBL_DECLARE_TAG;
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
    bool working;
};


struct sync_connection_s
{
    RBL_DECLARE_TAG;
    HttpMessageParser*               m_parser;
    IOBufferRef                  m_iobuffer;
    int                          socketfd;
    size_t                       read_buffer_size;
    /**
     * Next two fields are used for the on_message_callback to signal to the
     * read() function that a message has/has not been received.
     */
    //HttpMessageRef                   input_message_ref; //NULL when no messages received
    ListRef                      input_list;
    int                          reader_status;     // 1 indicates message received 0 otherwise

    /**
     * The next fields are the on_message_callback and its context pointer
     * There are only two cases in this application so rather than use
     * void* to annonymize things I decided to use a union to
     * overlay the two choices
     */
#if 0
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
#endif
};

struct sync_client_s {
    RBL_DECLARE_TAG;
    void*   user_ptr;
    size_t read_buffer_size;
    sync_connection_t* connection_ptr;
    pthread_mutex_t mutex;
};


#endif