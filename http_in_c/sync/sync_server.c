
#define CHLOG_ON
#include <http_in_c/sync/sync.h>
#include <http_in_c/sync/sync_internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <rbl/logger.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/common/queue.h>

#include <rbl/check_tag.h>

#define MAX_THREADS 100


//
// create a listening socket from host and port
//
socket_handle_t create_listener_socket(int port, const char* host)
{

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socket_handle_t tmp_socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if( (tmp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ) 
        goto error_01;

    // sin.sin_len = sizeof(sin);
    if( (result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0 )
        goto error_02;
    if( (result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes))) != 0 )
        goto error_02;

    if( (result = bind(tmp_socket, (struct sockaddr *)&sin, sizeof(sin))) != 0)
        goto error_03;

    if((result = listen(tmp_socket, SOMAXCONN)) != 0)
        goto error_04;
    return tmp_socket;

    error_01:
        RBL_LOG_ERROR("socket call failed with errno %d \n", errno);
        assert(0);
    error_02:
        RBL_LOG_ERROR("setsockopt call failed with errno %d \n", errno);
        assert(0);
    error_03:
        RBL_LOG_ERROR("bind call failed with errno %d \n", errno);
        assert(0);
    error_04:
        RBL_LOG_ERROR("listen call failed with errno %d \n", errno);
        assert(0);
}

sync_server_r sync_server_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler)
{
    sync_server_r sref = (sync_server_r)eg_alloc(sizeof(sync_server_t));
    sref->nbr_workers = nbr_threads;
    sref->port = port;
    sref->app_handler = app_handler;
#ifdef SYNC_WORKER_QUEUE
    sref->qref = Queue_new();
#endif

#ifdef DYN_WORKER_TAB
    sref->worker_tab = malloc(sizeof(sync_worker_r) * nbr_threads);
#else
    assert(nbr_threads < MAX_THREADS);
#endif
    RBL_SET_TAG(SYNC_SERVER_TAG, sref)
    return sref;
}

void sync_server_dispose(sync_server_r* srefptr)
{
    ASSERT_NOT_NULL(*srefptr);
    RBL_CHECK_TAG(SYNC_SERVER_TAG, *srefptr)
    free(*srefptr);
    *srefptr = NULL;
}

void sync_server_listen(sync_server_r server)
{
    ASSERT_NOT_NULL(server)
    RBL_SET_TAG(SYNC_SERVER_TAG, server)
    RBL_LOG_FMT("sync_server_listen");
    /**
     * Create the listening socket and bind it to localhost:port
     */
    int port = server->port;
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int)sizeof(peername);
    server->socket_fd = create_listener_socket(port, "127.0.0.1");
    //
    // Start the worker threads
    //
    for(int i = 0; i < server->nbr_workers; i++)
    {
#ifdef SYNC_WORKER_QUEUE
        RBL_LOGFMT("sync_server_listen SYNC_WORKER_QUEUE is defined");
        sync_worker_r wref = sync_worker_new(server->qref, i, server->read_buffer_size, server->app_handler);
#else
        RBL_LOGFMT("sync_server_listen SYNC_WORKER_QUEUE is NOT defined");
        sync_worker_r wref = sync_worker_new(server->socket_fd, i, server->read_buffer_size, server->app_handler);
#endif
        server->worker_tab[i] = NULL;
        if(sync_worker_start(wref) != 0) {
            RBL_LOG_ERROR("sync_server_t failed starting thread - aborting");
            return;
        }
        server->worker_tab[i] = wref;
    }

#ifdef SYNC_WORKER_QUEUE
    // now listen  for incoming connections
    {
        for(;;)
        {
            int sock2 = accept(server->socket_fd, (struct sockaddr*)&peername, &addr_length);
            if( sock2 <= 0 )
            {
                RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
                break;
            }
            RBL_LOG_FMT("SyncServer_listener adding socket to qref %d queue size: %ld queue capacity %ld ", sock2, Queue_size(server->qref),
                    Queue_capacity(server->qref));
            Queue_add(server->qref, sock2);
        }
    }
#endif
    RBL_LOG_FMT("About to join all threads\n");
    //
    // wait for the workers to complete
    //
    for(int i = 0; i < server->nbr_workers; i++) {
        sync_worker_r wref = server->worker_tab[i];
        if(wref != NULL) {
            RBL_LOG_FMT("About to joined worker %d\n", i);
            sync_worker_join(wref);
            sync_worker_dispose(wref);
        }
    }
    Queue_dispose(&(server->qref));
    // also wait for the monitor  to complete
    
}
void sync_server_terminate(sync_server_r this)
{
    ASSERT_NOT_NULL(this)
    RBL_CHECK_TAG(SYNC_SERVER_TAG, this)
    for(int i = 0; i < this->nbr_workers; i++) {
        Queue_add(this->qref, -1);
    }
    close(this->socket_fd);
}

