#ifndef c_http_tests_test_reactor_listener_h
#define c_http_tests_test_reactor_listener_h

#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include <http_in_c/async-old/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <http_in_c/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/sync/sync_client.h>
#include <http_in_c/runloop/runloop.h>

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
struct Listener_s {
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    ReactorRef              reactor_ref;
    RtorListenerRef         listening_watcher_ref;
    RtorTimerRef            timer_ref;
    TcpConnListRef          conn_list_ref;
    int                     listen_counter;
    int                     accept_count;
};
typedef struct  Listener_s TestServer, *ListenerRef;


ListenerRef Listener_new(int listen_fd);
ListenerRef Listener_init(ListenerRef sref, int listen_fd);
void Listener_dispose(ListenerRef *sref);
void Listener_listen(ListenerRef sref);

socket_handle_t create_listener_socket(int port, const char *host);
void set_non_blocking(socket_handle_t socket);


#endif