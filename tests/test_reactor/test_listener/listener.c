
#define XR_TRACE_ENABLE
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include "listener.h"
//#include <http_in_c/async/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <http_in_c/unittest.h>
#include <http_in_c/logger.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/sync/sync_client.h>


static void on_event_listening(RtorListenerRef listener_ref, uint64_t event);
static void on_timer(RtorTimerRef watcher, EventMask event);


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
    sref->reactor_ref = rtor_reactor_new();
    sref->listening_watcher_ref = rtor_listener_new(sref->reactor_ref, sref->listening_socket_fd);
    RtorListenerRef lw = sref->listening_watcher_ref;

    rtor_listener_register(lw, on_event_listening, sref);
    printf("Listener_listen reactor: %p listen sock: %d  lw: %p\n", sref->reactor_ref, sref->listening_socket_fd, lw);
    sref->timer_ref = rtor_timer_new(sref->reactor_ref);
    rtor_timer_register(sref->timer_ref, &on_timer, (void *) sref, 5000, false);
    rtor_reactor_run(sref->reactor_ref, -1);
    printf("Listener reactor ended \n");
    rtor_reactor_free(sref->reactor_ref);
}
/**
 * When the timer fires it is time to kill the listener.
 */
static void on_timer(RtorTimerRef watcher, EventMask event)
{
    printf("on_timer entered \n");
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    ListenerRef listener_ref = (ListenerRef) watcher->timer_handler_arg;
    LOG_FMT("event is : %lx  EPOLLIN: %ld  EPOLLERR: %ld", event, epollin, error);
    rtor_reactor_close(listener_ref->reactor_ref);
//    rtor_reactor_free(listener_ref->reactor_ref);
}

static void on_event_listening(RtorListenerRef listener_ref, uint64_t event)
{
    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    ListenerRef server_ref = listener_ref->listen_arg;
    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        int errno_saved = errno;
        LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        printf("Sock2 successfull sock: %d server_ref %p listen_ref: %p  listen_count: %d\n", sock2, server_ref, listener_ref, server_ref->listen_counter);
        if(server_ref->listen_counter == 0) {
            // pretend to be busy
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
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    assert(fres == 0);
}

