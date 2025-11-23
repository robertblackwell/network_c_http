

#include <netdb.h>
#include <pthread.h>
#include <src/common/cbuffer.h>
#include <src/http/http_message.h>
#include <rbl/logger.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "sync_client.h"

#define sync_client_TAG "SYCLNT"
#include <rbl/check_tag.h>
#include <pthread.h>

sync_client_t* sync_client_new(size_t read_buffer_size)
{
    sync_client_t* this = malloc(sizeof(sync_client_t));
    RBL_SET_TAG(sync_client_TAG, this)
    this->user_ptr = NULL;
    return this;
}
void sync_client_init(sync_client_t* this, size_t read_buffer_size)
{
    this->user_ptr = NULL;
    this->read_buffer_size = read_buffer_size;
    if(pthread_mutex_init(&this->mutex, NULL) != 0) {
        RBL_LOG_FMT("sync_client_init mutex init failed");
        exit(-1);
    };
}
void sync_client_free(sync_client_t* clientptr)
{
    RBL_CHECK_TAG(sync_client_TAG, clientptr)
    if (clientptr->user_ptr != (void*)-1) close(clientptr->socket_fd);
    free(clientptr);
}
static void connection_helper(sync_client_t* this, char* host, int portno)
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
        if(connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            // this->connection_ptr = sync_connection_new(sfd, this->read_buffer_size);
            this->socket_fd = sfd;
            break; // success
        }
        close(sfd);
    }
    if(rp == NULL) {
        RBL_LOG_FMT("Could not bind");
        exit(-1);
    }
    freeaddrinfo(result);
}
void sync_client_connect(sync_client_t* this, char* host, int portno)//, SyncAppMessageHandler handler)
{
    RBL_CHECK_TAG(sync_client_TAG, this)
    connection_helper(this, host, portno);
}
void sync_client_close(sync_client_t* this)
{
    RBL_CHECK_TAG(sync_client_TAG, this)
    close(this->socket_fd);
    // sync_connection_close(this->connection_ptr);
}
void* sync_client_get_userptr(sync_client_t* this)
{
    RBL_CHECK_TAG(sync_client_TAG, this)
    return this->user_ptr;
}
void sync_client_set_userptr(sync_client_t* this, void* userptr)
{
    RBL_CHECK_TAG(sync_client_TAG, this)
    this->user_ptr = userptr;
}