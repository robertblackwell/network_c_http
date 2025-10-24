
#include <src/async/async_socket.h>
#include <src/async/async.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <src/common/alloc.h>
#include <src/common/utils.h>
#include <rbl/macros.h>
#include <rbl/check_tag.h>

static AsyncHandlerRef my_only_client;

int async_create_shareable_socket()
{
    int sock = async_socket_create();
    async_socket_set_reuseaddr(sock);
    async_socket_set_reuseport(sock);
    async_socket_set_nonblocking(sock);
    RBL_ASSERT((sock > 0), "check valid shareable socket");
    return sock;
}
int async_socket_create()
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set non blocking error - error %d %s", errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((sock != -1), "async create socket failed");
}
void async_socket_set_reuseaddr(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set reuseaddr error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "set_socket_reuseaddr failed");
}
void async_socket_set_reuseport(int socket)
{
    int yes = 1;
    int result = setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set reuseport error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "set_socket_reuseaddr failed");
}
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, modFlags2);
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set non blocking error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "set socket non blocking");
}
void async_socket_listen(int socket)
{
    int result = listen(socket, SOMAXCONN);
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("listen error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "async socket listen failed");
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
        RBL_LOG_ERROR("bind error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "async socket bind failed");
}
