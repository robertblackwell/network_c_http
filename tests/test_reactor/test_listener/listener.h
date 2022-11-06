#ifndef c_http_tests_test_reactor_listener_h
#define c_http_tests_test_reactor_listener_h

#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include <c_http/async/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <c_http/unittest.h>
#include <c_http/common/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/sync/sync_client.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/w_timerfd.h>
#include <c_http/runloop/w_iofd.h>
#include <c_http/runloop/w_listener.h>
#include <c_http/runloop/w_eventfd.h>

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
    ReactorRef            reactor_ref;
    WListenerFdRef            listening_watcher_ref;
    WTimerFdRef               timer_ref;
    TcpConnListRef           conn_list_ref;
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