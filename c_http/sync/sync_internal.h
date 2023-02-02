#ifndef c_http_sync_sync_internal_h
#define c_http_sync_sync_internal_h
#include <c_http/logger.h>
#include <c_http/socket_functions.h>
#include <c_http/common/queue.h>
#include <c_http/http_parser/ll_parser.h>

#include <c_http/sync/sync.h>

#define SyncServer_TAG "SYNCSVER"
#include <c_http/check_tag.h>


#define MAX_THREADS 100
#define XDYN_WORKER_TAB
struct SyncServer_s {
    DECLARE_TAG;
    int                         port;
    size_t                      read_buffer_size;
    socket_handle_t             socket_fd;
    int                         nbr_workers;
    SyncAppMessageHandler       app_handler;
    QueueRef                    qref;
#ifdef DYN_WORKER_TAB
    WorkerRef                   *worker_tab;
#else
    WorkerRef                   worker_tab[MAX_THREADS];
#endif
};
#define Worker_TAG "SYNCWRKR"
#include <c_http/check_tag.h>

struct Worker_s {
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
#define sync_connection_TAG "SYNCCONN"
#include <c_http/check_tag.h>

struct sync_connection_s
{
    DECLARE_TAG;
    http_parser_t*              m_parser;
    IOBufferRef                 m_iobuffer;
    int                         socketfd;
    size_t                      read_buffer_size;
    SyncConnectionMessageHandler    handler;
    void*                       handler_context;
    int                 m_io_errno;
    int                 m_http_errno;
    char*               m_http_err_name;
    char*               m_http_err_description;
};


#endif