#define _GNU_SOURCE

#include <c_http/xr/xr_server.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <c_http/constants.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>

#include <c_http/logger.h>
#include <c_http/socket_functions.h>
#include <c_http/xr/evfd_queue.h>
#include<c_http/xr/conn.h>
#include <c_http/xr/w_socket.h>
#include <sys/epoll.h>


//
// create a listening socket from host and port
//
static socket_handle_t create_listener_socket(int port, const char *host)
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

static void on_post_done(void* arg)
{
    XrConnRef conn_ref = arg;
    XR_TRACE("conn: %p arg: %p", conn_ref, arg);
}
static void on_cb_message(XrConnRef conn_ref, void* arg, int status)
{
    XR_TRACE("conn: %p arg: %p status: %d", conn_ref, arg, status);
    assert(conn_ref->handler_ref == NULL);
    XrHandler_function(conn_ref->req_msg_ref, conn_ref, &on_post_done);
}
void on_event_listening(WSocketRef socket_watcher_ref, void *arg, uint64_t event)
{

    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    XrServerRef server_ref = arg;
    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    WSocketRef sw_ref = WSocket_new(server_ref->reactor_ref, sock2);
    XrConnRef conn = XrConn_new(sock2, sw_ref, server_ref);
    MessageRef inmsg = Message_new();
    XrConn_read_msg(conn, inmsg, on_cb_message, (void*)conn);
    return;
}

XrServerRef XrServer_new(int port)
{
    XrServerRef sref = (XrServerRef) eg_alloc(sizeof(XrServer));
    sref->port = port;
    return sref;
}

void XrServer_free(XrServerRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}

void XrServer_listen(XrServerRef sref)
{
    ASSERT_NOT_NULL(sref)
    int port = sref->port;
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    sref->listening_socket_fd = create_listener_socket(port, "127.0.0.1");
    set_non_blocking(sref->listening_socket_fd);
    sref->reactor_ref = XrReactor_new();
    sref->listening_watcher_ref = WSocket_new(sref->reactor_ref, sref->listening_socket_fd);
    WSocketRef lw = sref->listening_watcher_ref;
    uint64_t interest = EPOLLIN | EPOLLERR;
    WSocket_register(lw);
    WSocket_arm_read(lw, on_event_listening, sref);
    XrReactor_run(sref->reactor_ref, -1);
}

void XrServer_terminate(XrServerRef this)
{
    close(this->listening_socket_fd);
}

