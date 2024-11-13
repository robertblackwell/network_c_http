
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
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/common/queue.h>

#include <rbl/check_tag.h>

#define MAX_THREADS 100

sync_server_r sync_server_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler)
{
    sync_server_r sref = (sync_server_r)eg_alloc(sizeof(sync_server_t));
    sref->nbr_workers = nbr_threads;
    sref->port = port;
    sref->app_handler = app_handler;
    assert(nbr_threads < MAX_THREADS);
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
        RBL_LOG_FMT("sync_server_listen SYNC_WORKER_QUEUE is defined");
        sync_worker_r wref = sync_worker_new(server->qref, i, server->read_buffer_size, server->app_handler);
#else
        RBL_LOG_FMT("sync_server_listen SYNC_WORKER_QUEUE is NOT defined");
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
    Queue_free(server->qref);
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

