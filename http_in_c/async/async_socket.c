
#define CHLOG_ON
#include <http_in_c/async/async.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <http_in_c/logger.h>
#include <http_in_c/macros.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>

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


//void on_event_listening(RtorListenerRef listener_watcher_ref, uint64_t event);
//static void on_handler_completion_cb(AsyncServerRef sref, AsyncHandlerRef handler_ref)
//{
//    LOG_FMT("file: async_server.c on_handler_completion_cb");
//
//    AsyncServerRef server_ref = sref;
//    CHECK_TAG(AsyncServer_TAG, server_ref)
//    ListIter x = List_find(server_ref->handler_list, handler_ref);
//    AsyncHandlerRef href = List_itr_unpack(server_ref->handler_list, x);
//    List_itr_remove(server_ref->handler_list, &x);
//}
//void AsyncServer_init_with_socket(AsyncServerRef sref, int port, const char* host, int listen_socket, AsyncProcessRequestFunction process_request)
//{
//    SET_TAG(AsyncServer_TAG, sref)
//    sref->handler_complete = &on_handler_completion_cb;
//    sref->process_request = process_request;
//    sref->port = port;
//    sref->reactor_ref = rtor_reactor_new();
//    sref->listening_socket_fd = listen_socket;
//    sref->listening_watcher_ref = rtor_listener_new(sref->reactor_ref, sref->listening_socket_fd);
//    // TODO this is a memory leak
//    // the list is the owner of handler references
//    sref->handler_list = List_new(async_handler_anonymous_dispose);
//    async_socket_bind(sref->listening_socket_fd, port, host);
//    // do not call listen yet the reactor is not running
////    async_socket_listen(sref->listening_socket_fd);
//}
//
//void AsyncServer_init(AsyncServerRef sref, int port, const char* host, AsyncProcessRequestFunction process_request)
//{
//    SET_TAG(AsyncServer_TAG, sref)
//    int listening_socket_fd = async_create_shareable_socket();
//    AsyncServer_init_with_socket(sref, port, host, listening_socket_fd, process_request);
//
//}
//AsyncServerRef AsyncServer_new_with_socket(int port, const char* host, int listen_socket ,AsyncProcessRequestFunction process_request)
//{
//    AsyncServerRef sref = (AsyncServerRef) malloc(sizeof(AsyncServer));
//    AsyncServer_init_with_socket(sref, port, host, listen_socket, process_request);
//    return sref;
//}
//AsyncServerRef AsyncServer_new(int port, const char* host, AsyncProcessRequestFunction process_request)
//{
//    AsyncServerRef sref = (AsyncServerRef) malloc(sizeof(AsyncServer));
//    AsyncServer_init(sref, port, host, process_request);
//    return sref;
//}
//void AsyncServer_destroy(AsyncServerRef this)
//{
//    CHECK_TAG(AsyncServer_TAG, this)
//    ASSERT_NOT_NULL(this);
//    rtor_listener_deregister(this->listening_watcher_ref);
//    rtor_listener_free(this->listening_watcher_ref);
//    close(this->listening_socket_fd);
//    rtor_reactor_free(this->reactor_ref);
//    List_dispose(&(this->handler_list));
//    INVALIDATE_TAG(this)
//    // INVALIDATE_STRUCT(this, AsyncServer)
//}
//void AsyncServer_free(AsyncServerRef this)
//{
//    CHECK_TAG(AsyncServer_TAG, this)
//    ASSERT_NOT_NULL(this);
//    AsyncServer_destroy(this);
////    rtor_listener_deregister(this->listening_watcher_ref);
////    rtor_listener_free(this->listening_watcher_ref);
////    close(this->listening_socket_fd);
////    rtor_reactor_free(this->reactor_ref);
////    List_dispose(&(this->handler_list));
//    free(this);
//
//}
//void AsyncServer_dispose(AsyncServerRef *sref)
//{
//    CHECK_TAG(AsyncServer_TAG, *sref)
//    ASSERT_NOT_NULL(*sref);
//    free(*sref);
//    *sref = NULL;
//}
//void AsyncServer_listen(AsyncServerRef sref)
//{
//    CHECK_TAG(AsyncServer_TAG, sref)
//    ASSERT_NOT_NULL(sref)
//    int port = sref->port;
//    struct sockaddr_in peername;
//    unsigned int addr_length = (unsigned int) sizeof(peername);
//    RtorListenerRef lw = sref->listening_watcher_ref;
//    rtor_listener_register(lw, on_event_listening, sref);
//    async_socket_listen(sref->listening_socket_fd);
//    rtor_reactor_run(sref->reactor_ref, -1);
//    LOG_FMT("DemoServer finishing");
//
//}
//void DemoServer_terminate(AsyncServerRef this)
//{
//    CHECK_TAG(AsyncServer_TAG, this)
//
//    close(this->listening_socket_fd);
//}

//
// create a listening socket from host and port
//
int async_create_shareable_socket()
{
    int sock = async_socket_create();
    async_socket_set_reuseaddr(sock);
    async_socket_set_reuseport(sock);
    async_socket_set_nonblocking(sock);
    CHTTP_ASSERT((sock > 0),"check valid shareable socket");
    return sock;
}
int async_socket_create()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1) {
        int errno_saved = errno;
        LOG_ERROR("set non blocking error - error %d %s", errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((sock != -1),"async create socket failed");
}
void async_socket_set_reuseaddr(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(result != 0) {
        int errno_saved = errno;
        LOG_ERROR("set reuseaddr error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((result == 0),"set_socket_reuseaddr failed");
}
void async_socket_set_reuseport(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
    if(result != 0) {
        int errno_saved = errno;
        LOG_ERROR("set reuseport error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((result == 0),"set_socket_reuseaddr failed");
}
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, modFlags2);
    if(result != 0) {
        int errno_saved = errno;
        LOG_ERROR("set non blocking error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((result == 0), "set socket non blocking");
}
void async_socket_listen(int socket)
{
    int result = listen(socket, SOMAXCONN);
    if(result != 0) {
        int errno_saved = errno;
        LOG_ERROR("listen error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((result == 0),"async socket listen failed");
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
    if(result != 0) {
        int errno_saved = errno;
        LOG_ERROR("bind error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    CHTTP_ASSERT((result == 0), "async socket bind failed");
}

//void on_event_listening(RtorListenerRef listener_watcher_ref, uint64_t event)
//{
//    LOG_FMT("listening_hander");
//    AsyncServerRef server_ref = listener_watcher_ref->listen_arg;
//    CHECK_TAG(AsyncServer_TAG, server_ref)
//    struct sockaddr_in peername;
//    unsigned int addr_length = (unsigned int) sizeof(peername);
//
//    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
//    LOGFMT("new socket %d", sock2);
//    if(sock2 <= 0) {
//        LOG_ERROR("accpt failed errno %d  strerror: %s", errno, strerror(errno));
//        LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
//    }
//    LOG_FMT("Listerner accepted sock fd: %d", sock2);
//    printf("async on listening new socket %d\n", sock2);
//    async_socket_set_nonblocking(sock2);
//    AsyncHandlerRef handler = async_handler_new(
//            sock2,
//            rtor_listener_get_reactor(listener_watcher_ref),
//            server_ref);
//
//    List_add_back(server_ref->handler_list, handler);
//}
//
