#include "connector_ctx.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include <rbl/logger.h>
int connection_helper_v2(int port)
{
    struct sockaddr_in server;
    int lfd;
    char r_buff[100] = "";
    char s_buff[100] = "";
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    int r = connect(lfd, (struct sockaddr *)&server, sizeof(server));
    int errno_saved = errno; 
    if(r == 0) {
        // this->connection_ptr = sync_connection_new(sfd, this->read_buffer_size);
        return lfd;
    } else {
        printf("connect failed errno: %d desc: %s\n", errno_saved, strerror(errno_saved));
        assert(false);
    }

}
int connection_helper(char* host, int portno)
//https://linux.die.net/man/3/getaddrinfo
// this function exists as a hackish way to replace the code that resolves host names
// and connects. The original code to do this (see #ifdef'd out) was using gethostbyname()
// which is not safe in a multi-thread environment and in any case is deprecated
{
#define NC_BUF_SIZE 500
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    size_t nread;
    char buf[NC_BUF_SIZE];
    char portstr[100];
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    sprintf(portstr, "%d", portno);

    int errc = getaddrinfo(host, portstr, &hints, &result);
    if(errc != 0) {
        RBL_LOG_ERROR("getaddrinfo : %s", gai_strerror(errc));
        exit(-1);
    }
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1) {
            continue;
        }
        int r = connect(sfd, rp->ai_addr, rp->ai_addrlen);
        int errno_saved = errno; 
        if(r == 0) {
            // this->connection_ptr = sync_connection_new(sfd, this->read_buffer_size);
            break; // success
        } else {
            printf("connect failed errno: %d desc: %s\n", errno_saved, strerror(errno_saved));
            assert(false);
        }
        close(sfd);
    }
    if(rp == NULL) {
        RBL_LOG_FMT("Could not bind");
        exit(-1);
    }
    freeaddrinfo(result);
    return 0;
}

void* connector_thread_func(void* arg)
{
    Connector* tc = (Connector*)arg;
    for(int i = 0; i < tc->max_count; i++) {
        printf("Client about to connect %d \n", i);
        int sock = connection_helper_v2(tc->port);
        assert(sock > 0);
        sleep(2);
        tc->count++;
        int fd = socket_is_blocking(sock);
        printf("connector write to listener\n");
        char* b = "Hello from connector";
        int wn = write(sock, b, strlen(b));
        assert(wn > 0);
        sleep(100);
        char buffer[200];
        int rn = read(wn, buffer, 100);
        printf("connector read rn: %d\n", rn);
        assert(rn > 0);
        buffer[rn] = '\0';
        printf("connector received %s\n", buffer);
        usleep(200000);
    }
    printf("Connector loop ended \n");
    // now wait here for all connections to be processed
    sleep(1);
    // for(int i = 0; i < 2; i++) {
    //     TestAsyncServerRef server = tc->servers[i];
    //     RunloopListenerRef listener = server->listening_watcher_ref;
    //     runloop_listener_deregister(listener);
    // }
    return NULL;
}
