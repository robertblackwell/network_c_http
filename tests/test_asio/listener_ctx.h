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
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include <src/sync/sync_client.h>
#include <src/runloop/runloop.h>

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
    int                     port;
    const char*             host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopEventRef         listening_watcher_ref;
    RunloopEventRef         timer_ref;
    AsioListenerRef         asio_listener_ref;
    int                     listen_count;
    int                     accept_count;
    int id;
};
typedef struct  ListenerCtx_s TestServer, *ListenerCtxRef;


ListenerCtxRef listener_ctx_new(int listen_fd, int id);
void listener_ctx_init(ListenerCtxRef sref, int listen_fd, int id);

ListenerCtxRef listener_ctx_new2(int port, const char* host, int id);
void listener_ctx_init2(ListenerCtxRef sref, int port, const char* host, int id);


void listener_ctx_free(ListenerCtxRef *sref);
void listener_ctx_listen(ListenerCtxRef sref);

int local_create_bound_socket(int port, const char* host);


#endif