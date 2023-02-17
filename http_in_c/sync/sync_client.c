

#include <netdb.h>
#include <pthread.h>
#include <http_in_c/sync/sync.h>
#include <http_in_c/sync/sync_internal.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/cbuffer.h>
#include <http_in_c/http/message.h>
#include <http_in_c/logger.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define sync_client_TAG "SYCLNT"
#include <http_in_c/check_tag.h>
#include <pthread.h>

sync_client_t* sync_client_new(size_t read_buffer_size)
{
    sync_client_t* this = eg_alloc(sizeof(sync_client_t));
    SET_TAG(sync_client_TAG, this)
    this->connection_ptr = NULL;
    this->user_ptr = NULL;
}
void sync_client_init(sync_client_t* this, size_t read_buffer_size)
{
    this->connection_ptr = NULL;
    this->user_ptr = NULL;
    this->read_buffer_size = read_buffer_size;
    if(pthread_mutex_init(&this->mutex, NULL) != 0) {
        LOGFMT("sync_client_init mutex init failed");
        exit(-1);
    };
}
void sync_client_destroy(sync_client_t* this)
{
    CHECK_TAG(sync_client_TAG, this)
    LOG_FMT("sync_client_dispose %p  %d\n", this, this->socketfd);
    if(this->connection_ptr) sync_connection_dispose(&(this->connection_ptr));
    INVALIDATE_TAG(this)
    // INVALIDATE_STRUCT(this, sync_client_t)
}
void sync_client_dispose(sync_client_t** this_ptr)
{
    sync_client_t* this = *this_ptr;
    CHECK_TAG(sync_client_TAG, this)
    sync_client_destroy(this);
    eg_free(*this_ptr);
    // in here add code to obliterate the structure
    *this_ptr = NULL;
}
#if 0
static struct hostent* _gethostname(char* host)
{
    struct hostent *hostbuf, *hp;
    size_t hostbuflen;
    char *tmphostbuf;
    int res;
    int herr;

    hostbuf = malloc(sizeof(struct hostent));
    hostbuflen = 1024;
    tmphostbuf = malloc(hostbuflen);

    while((res = gethostbyname_r(host, tmphostbuf, hostbuflen, &hp, &herr)) == ERANGE)
    {
        /* Enlarge the buffer */
        tmphostbuf = reallocarray(tmphostbuf, hostbuflen, 2);
        hostbuflen *= 2;
    }
    free(tmphostbuf);
    if(res || hp == NULL)
        return NULL;
    return hp;
}
#endif
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
        LOG_ERROR("getaddrinfo : %s", gai_strerror(errc));
        exit(-1);
    }
    for(rp = result; rp != NULL; rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1) {
            continue;
        }
        if(connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            this->connection_ptr = sync_connection_new(sfd, this->read_buffer_size);
            break; // success
        }
        close(sfd);
    }
    if(rp == NULL) {
        LOGFMT("Could not bind");
        exit(-1);
    }
    freeaddrinfo(result);
}
void sync_client_connect(sync_client_t* this, char* host, int portno)//, SyncAppMessageHandler handler)
{
    CHECK_TAG(sync_client_TAG, this)
    connection_helper(this, host, portno);
    return;
#if 0
    int sockfd, n;
    SET_TAG(sync_client_TAG, this)
    struct sockaddr_in serv_addr;
    struct hostent *hostent_ptr;

    pthread_mutex_lock(&this->mutex);
    printf("sync_client connect - this: %p host: %s port: %d\n", this, host, portno);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("sync_client socketfd: %d\n", sockfd);
    if (sockfd < 0) {
        printf("socket call failed ");
        LOG_ERROR("ERROR opening socket");
        exit(-1);
    }
    hostent_ptr = gethostbyname(host);
    printf("sync_client hostent %p\n", hostent_ptr);
    if (hostent_ptr == NULL) {
        printf("sync_client connect ERROR, no such host: %s \n", host);
        exit(-1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)hostent_ptr->h_addr, (char *)&serv_addr.sin_addr.s_addr, hostent_ptr->h_length);
    serv_addr.sin_port = htons(portno);
    LOG_FMT("sync_client_connect %p sockfd: %d\n", this, sockfd);
    int cstatus;
    cstatus = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (cstatus  < 0) {
        int errno_saved = errno;
        printf("connect failed socket: %d cstatus: %d errno_saved %d %s\n", sockfd, cstatus, errno_saved, strerror(errno_saved));
        LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
        exit(-1);
    }
    pthread_mutex_unlock(&this->mutex);
    this->connection_ptr = sync_connection_new(sockfd, this->read_buffer_size);
#endif

}
//void sync_client_request_round_trip(sync_client_t* this, MessageRef request_ref, SyncConnectionClientMessageHandler handler)
//{
//    CHECK_TAG(sync_client_TAG, this)
//    sync_connection_write(this->connection_ptr, request_ref);
//
//    int rc = sync_connection_read_response(this->connection_ptr, handler, this);
//    llhttp_errno_t en = rc;
//    return;
//}
void sync_client_close(sync_client_t* this)
{
    sync_connection_close(this->connection_ptr);
}
void* sync_client_get_userptr(sync_client_t* this)
{
    CHECK_TAG(sync_client_TAG, this)
    return this->user_ptr;
}
void sync_client_set_userptr(sync_client_t* this, void* userptr)
{
    CHECK_TAG(sync_client_TAG, this)
    this->user_ptr = userptr;
}