
#define CHLOG_ON
#include <http_in_c/async/async.h>
#include <http_in_c/async/async_internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>

#if 0
static AsyncHandlerRef my_only_client;

int async_create_shareable_socket();
int async_socket_create();
void async_socket_set_nonblocking(int socket);
void async_socket_set_reuseaddr(int socket);
void async_socket_set_reuseport(int socket);
void async_socket_set_nonblocking(int socket);
void async_socket_listen(int socket);
void async_socket_bind(int socket, int port, const char* host);
int async_bind_and_listen_socket(int socket, int port, const char *host);
#endif

void on_event_listening(RunloopListenerRef listener_watcher_ref, uint64_t event);
static void on_handler_completion_cb(AsyncServerRef sref, AsyncHandlerRef handler_ref)
{
    RBL_LOG_FMT("file: async_server.c on_handler_completion_cb");

    AsyncServerRef server_ref = sref;
    RBL_CHECK_TAG(AsyncServer_TAG, server_ref)
    ListIter x = List_find(server_ref->handler_list, handler_ref);
    AsyncHandlerRef href = List_itr_unpack(server_ref->handler_list, x);
    List_itr_remove(server_ref->handler_list, &x);
}
/**
 * THe listen_socket must be:
 *  -   created,
 *  -   set REUSEADDR,
 *  -   set REUSEPORT,
 *  -   set Non Blocking,
 *  -   bind() to port and host
 *
 *  BEFORE this call
 */
void AsyncServer_init_with_socket(AsyncServerRef sref, int port, const char* host, int listen_socket, AsyncProcessRequestFunction process_request)
{
    RBL_SET_TAG(AsyncServer_TAG, sref)
    sref->handler_complete = &on_handler_completion_cb;
    sref->process_request = process_request;
    sref->port = port;
    sref->reactor_ref = runloop_new();
    sref->listening_socket_fd = listen_socket;
    sref->listening_watcher_ref = runloop_listener_new(sref->reactor_ref, sref->listening_socket_fd);
    // TODO this is a memory leak
    // the list is the owner of handler references
    sref->handler_list = List_new(async_handler_anonymous_dispose);
}

void AsyncServer_init(AsyncServerRef sref, int port_number, const char* host, AsyncProcessRequestFunction process_request)
{
    RBL_SET_TAG(AsyncServer_TAG, sref)
    int listening_socket_fd = async_create_shareable_socket();
    async_socket_bind(listening_socket_fd, port_number, "127.0.0.1");
    async_socket_listen(listening_socket_fd);
    AsyncServer_init_with_socket(sref, port_number, host, listening_socket_fd, process_request);

}
AsyncServerRef AsyncServer_new_with_socket(int port, const char* host, int listen_socket ,AsyncProcessRequestFunction process_request)
{
    AsyncServerRef sref = (AsyncServerRef) malloc(sizeof(AsyncServer));
    AsyncServer_init_with_socket(sref, port, host, listen_socket, process_request);
    return sref;
}
AsyncServerRef AsyncServer_new(int port, const char* host, AsyncProcessRequestFunction process_request)
{
    AsyncServerRef sref = (AsyncServerRef) malloc(sizeof(AsyncServer));
    AsyncServer_init(sref, port, host, process_request);
    return sref;
}
void AsyncServer_destroy(AsyncServerRef this)
{
    RBL_CHECK_TAG(AsyncServer_TAG, this)
    ASSERT_NOT_NULL(this);
    runloop_listener_deregister(this->listening_watcher_ref);
    runloop_listener_free(this->listening_watcher_ref);
    close(this->listening_socket_fd);
    runloop_free(this->reactor_ref);
    List_dispose(&(this->handler_list));
    RBL_INVALIDATE_TAG(this)
    // RBL_INVALIDATE_STRUCT(this, AsyncServer)
}
void AsyncServer_free(AsyncServerRef this)
{
    RBL_CHECK_TAG(AsyncServer_TAG, this)
    ASSERT_NOT_NULL(this);
    AsyncServer_destroy(this);
//    runloop_listener_deregister(this->listening_watcher_ref);
//    runloop_listener_free(this->listening_watcher_ref);
//    close(this->listening_socket_fd);
//    runloop_free(this->reactor_ref);
//    List_dispose(&(this->handler_list));
    free(this);

}
void AsyncServer_dispose(AsyncServerRef *sref)
{
    RBL_CHECK_TAG(AsyncServer_TAG, *sref)
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}
void AsyncServer_start(AsyncServerRef sref)
{
    RBL_CHECK_TAG(AsyncServer_TAG, sref)
    ASSERT_NOT_NULL(sref)
    int port = sref->port;
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    RunloopListenerRef lw = sref->listening_watcher_ref;
    runloop_listener_register(lw, on_event_listening, sref);
    RBL_LOG_FMT("DemoServer finishing");
}
void DemoServer_terminate(AsyncServerRef this)
{
    RBL_CHECK_TAG(AsyncServer_TAG, this)

    close(this->listening_socket_fd);
}

#if 0
//
// create a listening socket from host and port
//
int async_create_shareable_socket()
{
    int sock = async_socket_create();
    async_socket_set_reuseaddr(sock);
    async_socket_set_reuseport(sock);
    async_socket_set_nonblocking(sock);
    return sock;
}
int async_socket_create()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    RBL_ASSERT((sock != -1),"async create socket failed");
}
void async_socket_set_reuseaddr(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    RBL_ASSERT((result == 0),"set_socket_reuseaddr failed");
}
void async_socket_set_reuseport(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
    RBL_ASSERT((result == 0),"set_socket_reuseaddr failed");
}
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    RBL_ASSERT((fres == 0), "set socket non blocking");
}
void async_socket_listen(int socket)
{
    int result = listen(socket, SOMAXCONN);
    RBL_ASSERT((result == 0),"async socket listen failed");
}
void async_socket_bind(int socket, int port, const char* host)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    int tmp_socket = socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int yes = 1;
    int result = bind(socket, (struct sockaddr *) &sin, sizeof(sin));
    RBL_ASSERT((result == 0), "async socket bind failed");
}
#endif

void on_event_listening(RunloopListenerRef listener_watcher_ref, uint64_t event)
{
    RBL_LOG_FMT("listening_hander");
    AsyncServerRef server_ref = listener_watcher_ref->listen_arg;
    RBL_CHECK_TAG(AsyncServer_TAG, server_ref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    RBL_LOG_FMT("new socket %d", sock2);
    if(sock2 <= 0) {
        int errno_saved = errno;
        if(errno_saved == EAGAIN || errno_saved == EWOULDBLOCK) {
            return;
        }
        RBL_LOG_ERROR("accpt failed socket: %d listening_socket_fd: %d errno %d  strerror: %s", sock2, server_ref->listening_socket_fd, errno_saved, strerror(errno_saved));
        RBL_ASSERT((0), "accept returns invalid socket");
        RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    RBL_LOG_FMT("Listerner accepted sock fd: %d", sock2);
    printf("async on listening new socket %d\n", sock2);
    async_socket_set_nonblocking(sock2);
    AsyncHandlerRef handler = async_handler_new(
            sock2,
            runloop_listener_get_reactor(listener_watcher_ref),
            server_ref);

    List_add_back(server_ref->handler_list, handler);
}

