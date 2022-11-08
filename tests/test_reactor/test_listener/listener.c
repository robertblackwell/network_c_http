#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include "listener.h"
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
#include <sys/epoll.h>
#include <c_http/unittest.h>
#include <c_http/logger.h>
#include <c_http/common/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/sync/sync_client.h>


static void on_event_listening(WListenerFdRef listener_ref, void *arg, uint64_t event);
static void on_timer(WTimerFdRef timer_ref, void *arg, EventMask event);


ListenerRef Listener_new(int listen_fd)
{
    ListenerRef sref = malloc(sizeof(TestServer));
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;
    printf("Listener_new %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}
ListenerRef Listener_init(ListenerRef sref, int listen_fd)
{
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;

    printf("Listener_init %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}

void Listener_dispose(ListenerRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}
void Listener_listen(ListenerRef sref)
{
    ASSERT_NOT_NULL(sref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    sref->reactor_ref = XrReactor_new();
    sref->listening_watcher_ref = WListenerFd_new(sref->reactor_ref, sref->listening_socket_fd);
    WListenerFdRef lw = sref->listening_watcher_ref;

    WListenerFd_register(lw, on_event_listening, sref);
    printf("Listener_listen reactor: %p listen sock: %d  lw: %p\n", sref->reactor_ref, sref->listening_socket_fd, lw);
    sref->timer_ref = WTimerFd_new(sref->reactor_ref, &on_timer, (void*)sref, 5000, false);
    XrReactor_run(sref->reactor_ref, -1);
}
/**
 * When the timer fires it is time to kill the listener.
 */
static void on_timer(WTimerFdRef watcher, void* ctx, EventMask event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    ListenerRef listener_ref = (ListenerRef) ctx;
    LOG_FMT("event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", event, epollin, error);
    XrReactor_free(listener_ref->reactor_ref);
}

static void on_event_listening(WListenerFdRef listener_ref, void *arg, uint64_t event)
{
//    assert(iobuf != NULL);
//    assert(conn_ref->handler_ref != NULL);
//    TcpConn_write(conn_ref, iobuf, &on_handler_write, arg);

    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    ListenerRef server_ref = arg;
    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        int errno_saved = errno;
        LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        printf("Sock2 successfull sock: %d server_ref %p listen_ref: %p  listen_count: %d\n", sock2, server_ref, listener_ref, server_ref->listen_counter);
        if(server_ref->listen_counter == 0) {
            sleep(1);
        }
        server_ref->listen_counter++;
        sleep(0.6);
    }
    close(sock2);
    printf("on_event_listen new socket is : %d\n", sock2);
}


socket_handle_t create_listener_socket(int port, const char *host)
{

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socket_handle_t tmp_socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if((tmp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        goto error_01;
    }

    // sin.sin_len = sizeof(sin);
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0) {
        goto error_02;
    }

    if((result = bind(tmp_socket, (struct sockaddr *) &sin, sizeof(sin))) != 0) {
        goto error_03;
    }

    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
        goto error_04;
    }
    return tmp_socket;

    error_01:
    printf("socket call failed with errno %d \n", errno);
    assert(0);
    error_02:
    printf("setsockopt call failed with errno %d \n", errno);
    assert(0);
    error_03:
    printf("bind call failed with errno %d \n", errno);
    assert(0);
    error_04:
    printf("listen call failed with errno %d \n", errno);
    assert(0);
}

void set_non_blocking(socket_handle_t socket)
{
    //
    // Ensure socket is in blocking mode
    //
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    assert(fres == 0);
}

