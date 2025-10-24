#include "listener_ctx.h"
#include <src/runloop/runloop.h>
// #include <src/runloop/rl_internal.h>
#include "listener_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>

static void on_timer(RunloopRef rl, void* arg);
static void on_accept(void* arg, int fd, int status);


ListenerCtxRef listener_ctx_new(int listen_fd)
{
    ListenerCtxRef sref = malloc(sizeof(TestServer));
    listener_ctx_init(sref, listen_fd);
    return sref;
}
void listener_ctx_init(ListenerCtxRef sref, int listen_fd)
{
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;
    printf("listener_ctx_init %p   listen fd: %d\n", sref, listen_fd);
}

ListenerCtxRef listener_ctx_new2(int port, const char* host)
{
    ListenerCtxRef sref = malloc(sizeof(TestServer));
    listener_ctx_init2(sref, port, host);
    return sref;
}
void listener_ctx_init2(ListenerCtxRef sref, int port, const char* host)
{
    sref->port = port;
    sref->host = host;
    sref->listen_counter = 0;
    sref->accept_count = 0;
}


void listener_ctx_free(ListenerCtxRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}
void listener_ctx_listen(ListenerCtxRef listener_ctx_ref)
{
    ASSERT_NOT_NULL(listener_ctx_ref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    listener_ctx_ref->runloop_ref = runloop_new();
#if 1
    int fd = local_create_bound_socket(listener_ctx_ref->port, listener_ctx_ref->host);
    socket_set_non_blocking(fd);
    listener_ctx_ref->listening_socket_fd = fd;
    int result;
    if((result = listen(fd, SOMAXCONN)) != 0) {
        printf("listen call failed with errno %d \n", errno);
        assert(0);
    }
    listener_ctx_ref->asio_listener_ref = asio_listener_new(listener_ctx_ref->runloop_ref, listener_ctx_ref->listening_socket_fd);
#else
//    listener_ctx_ref->asio_listener_ref = asio_listener_new_from_port_host(listener_ctx_ref->runloop_ref, listener_ctx_ref->port, listener_ctx_ref->host);
#endif

    listener_ctx_ref->timer_ref = runloop_timer_set(listener_ctx_ref->runloop_ref, on_timer, listener_ctx_ref, 5000, false);

    asio_accept(listener_ctx_ref->asio_listener_ref, &on_accept, listener_ctx_ref);
    runloop_run(listener_ctx_ref->runloop_ref, -1);
    printf("Listener reactor ended \n");
    runloop_free(listener_ctx_ref->runloop_ref);
}
void start_accept_again(ListenerCtxRef listener_ctx_ref)
{
    asio_accept(listener_ctx_ref->asio_listener_ref, &on_accept, listener_ctx_ref);
}
static void on_accept(void* ctx_arg, int accept_fd, int status)
{
    pid_t tid = gettid();
    ListenerCtxRef lctx_ref  = ctx_arg;
    printf("on_accept thread: %d ctx_arg: %p, fd: %d accept count: %d status: %d\n", tid, ctx_arg, accept_fd, lctx_ref->accept_count, status);
    if(lctx_ref->accept_count ==0) {
        sleep(1);
    } else {
        usleep(1000 * 500);
    }
    lctx_ref->listen_counter++;
    lctx_ref->accept_count++;
    start_accept_again(lctx_ref);
}

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
int local_create_bound_socket(int port, const char *host)
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
        printf("socket call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEPORT  , &yes, sizeof(yes))) != 0) {
        printf("setsockopt call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = bind(tmp_socket, (struct sockaddr *) &sin, sizeof(sin))) != 0) {
        printf("bind call failed with errno %d \n", errno);
        assert(0);
    }
//    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
//        printf("listen call failed with errno %d \n", errno);
//        assert(0);
//    }
    return tmp_socket;
}
