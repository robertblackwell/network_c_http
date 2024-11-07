
#define XR_TRACE_ENABLE
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>
#include "listener_ctx.h"
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
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/common/socket_functions.h>
#include <http_in_c/sync/sync_client.h>

#ifdef ASIO_LISTENER_DISABLED
    #ifdef ASIO_LISTENER_ENABLED
        #undef ASIO_LISTENER_ENABLED
    #endif
#endif
static void on_timer(RunloopRef rl, void* arg);
#ifdef ASIO_ENABLED
static void on_accept(void* arg, int fd, int status);
#else
static void on_event_listening(RunloopRef rl, void* listener_ref_arg);
#endif

ListenerCtxRef listener_ctx_new(int listen_fd)
{
    ListenerCtxRef sref = malloc(sizeof(TestServer));
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;
    printf("listener_ctx_new %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}
ListenerCtxRef listener_ctx_init(ListenerCtxRef sref, int listen_fd)
{
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;

    printf("listener_ctx_init %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}
void listener_ctx_dispose(ListenerCtxRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}
#ifdef ASIO_ENABLED
void listener_ctx_listen(ListenerCtxRef listener_ctx_ref)
{
    ASSERT_NOT_NULL(listener_ctx_ref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    listener_ctx_ref->runloop_ref = runloop_new();
    listener_ctx_ref->asio_listener_ref = asio_listener_new(listener_ctx_ref->runloop_ref, listener_ctx_ref->listening_socket_fd);
    listener_ctx_ref->timer_ref = runloop_timer_set(listener_ctx_ref->runloop_ref, on_timer, listener_ctx_ref, 5000, false);
    asio_listen(listener_ctx_ref->asio_listener_ref, &on_accept, listener_ctx_ref);
    runloop_run(listener_ctx_ref->runloop_ref, -1);
    printf("Listener reactor ended \n");
    runloop_free(listener_ctx_ref->runloop_ref);
}
static void on_accept(void* ctx_arg, int accept_fd, int status)
{
    pid_t tid = gettid();
    printf("on_accept thread: %d ctx_arg: %p, fd: %d status: %d\n", tid, ctx_arg, accept_fd, status);
    ListenerCtxRef lctx_ref  = ctx_arg;
    if(lctx_ref->accept_count ==0) {
        sleep(1);
    } else {
        usleep(1000 * 500);
    }
    lctx_ref->listen_counter++;
    lctx_ref->accept_count++;

}

#else
void listener_ctx_listen(ListenerCtxRef listener_ctx_ref)
{
    ASSERT_NOT_NULL(listener_ctx_ref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    listener_ctx_ref->runloop_ref = runloop_new();
    listener_ctx_ref->listening_watcher_ref = runloop_listener_new(listener_ctx_ref->runloop_ref, listener_ctx_ref->listening_socket_fd);
    RunloopListenerRef lw = listener_ctx_ref->listening_watcher_ref;

    runloop_listener_register(lw, on_event_listening, listener_ctx_ref);
    printf("listener_ctx_listen reactor: %p listen sock: %d  lw: %p\n", listener_ctx_ref->runloop_ref, listener_ctx_ref->listening_socket_fd, lw);
    listener_ctx_ref->timer_ref = runloop_timer_new(listener_ctx_ref->runloop_ref);
    runloop_timer_register(listener_ctx_ref->timer_ref, &on_timer, (void *) listener_ctx_ref, 5000, false);
    runloop_run(listener_ctx_ref->runloop_ref, -1);
    printf("Listener reactor ended \n");
    runloop_free(listener_ctx_ref->runloop_ref);
}
static void on_event_listening(RunloopRef rl, void* listener_ref_arg)
{
    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    ListenerCtxRef listener_ref  = listener_ref_arg;
    int sock2 = accept(listener_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        int errno_saved = errno;
        RBL_LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        printf("Sock2 successfull sock: %d server_ref %p listen_ref: %p  listen_count: %d\n", sock2, listener_ref, listener_ref, listener_ref->listen_counter);
        if(listener_ref->listen_counter == 0) {
            // pretend to be busy
            sleep(1);
        }
        listener_ref->listen_counter++;
        sleep(0.6);
    }
    close(sock2);
    printf("on_event_listen new socket is : %d\n", sock2);
}

#endif
/**
 * When the timer fires it is time to kill the listener.
 */
static void on_timer(RunloopRef rl, void* listener_ref_arg)
{
    printf("on_timer entered \n");
    ListenerCtxRef listener_ref = (ListenerCtxRef) listener_ref_arg;
    RBL_LOG_MSG("on_timer closing runloop ");
    runloop_close(listener_ref->runloop_ref);
//    runloop_free(listener_ref->reactor_ref);
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

