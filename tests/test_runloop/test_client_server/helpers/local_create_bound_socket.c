#include "local_create_bound_socket.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
typedef int socket_handle_t;
/**
 * This function creates a socket for listening and binds it to a host and port
 * BUT it does not issue the listen() call
 */
int local_create_bound_socket(int port, const char *host)
{
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    socket_handle_t tmp_socket;
    server.sin_family = AF_INET; // or AF_INET6 (address family)
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if((tmp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEPORT  , &yes, sizeof(yes))) != 0) {
        printf("setsockopt call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR  , &yes, sizeof(yes))) != 0) {
        printf("setsockopt call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = bind(tmp_socket, (struct sockaddr *) &server, sizeof(server))) != 0) {
        printf("bind call failed with errno %d \n", errno);
        assert(0);
    }
    //    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
    //        printf("listen call failed with errno %d \n", errno);
    //        assert(0);
    //    }
    return tmp_socket;
}
