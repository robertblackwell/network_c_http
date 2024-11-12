#ifndef c_http_tests_test_reactor_listener_h
#define c_http_tests_test_reactor_listener_h


#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/sync/sync_client.h>
#include <http_in_c/runloop/runloop.h>
#include "asio_listener.h"

typedef int socket_handle_t;

/**
 * Listener is an object that 
 * -    listens for an accepts connections, on a designated port.
 * -    holds accepted connection in a list.
 * -    terminates closing all connections after a specified period.
 * -    the timeinterval over which the object accepts connections is
 *      controlled by a w_timer
 * 
 */
struct ListenerCtx_s {
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopListenerRef      listening_watcher_ref;
    RunloopTimerRef         timer_ref;
    AsioListenerRef         asio_listener_ref;
    int                     listen_counter;
    int                     accept_count;
};
typedef struct  ListenerCtx_s TestServer, *ListenerCtxRef;


ListenerCtxRef listener_ctx_new(int listen_fd);
ListenerCtxRef listener_ctx_init(ListenerCtxRef sref, int listen_fd);
void listener_ctx_free(ListenerCtxRef *sref);
void listener_ctx_listen(ListenerCtxRef sref);

socket_handle_t local_create_listener_socket(int port, const char *host);
void set_non_blocking(socket_handle_t socket);


#endif