#include <c_http/api/client.h>
#include <c_http/dsl/alloc.h>
#include <c_http/api/cbuffer.h>
#include <c_http/api/message.h>
#include <c_http/api/reader.h>
#include <c_http/api/writer.h>
#include <c_http/logger.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define TYPE Client
#define Client_TAG "CLIENT"
#include <c_http/check_tag.h>
#undef TYPE
#define CLIENT_DECLARE_TAG DECLARE_TAG(Client)
#define CLIENT_CHECK_TAG(p) CHECK_TAG(Client, p)
#define CLIENT_SET_TAG(p) SET_TAG(Client, p)

struct Client_s {
    CLIENT_DECLARE_TAG;
    socket_handle_t sock;
    WriterRef wrtr;
    ReaderRef  rdr;
};

ClientRef Client_new()
{
    ClientRef this = eg_alloc(sizeof(Client));
    CLIENT_SET_TAG(this)
    this->rdr = NULL;
    this->wrtr = NULL;
}
void Client_dispose(ClientRef* this_ptr)
{
    ClientRef this = *this_ptr;
    CLIENT_CHECK_TAG(this)
    LOG_FMT("Client_dispose %p  %d\n", this, this->sock);
    if(this->rdr) Reader_dispose(&(this->rdr));
    if(this->wrtr) Writer_dispose(&(this->wrtr));
    close(this->sock);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}
void Client_raw_connect(ClientRef this, int sockfd, struct sockaddr* sockaddr_p, int sockaddr_len)
{
    LOG_FMT("Client_raw_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,sockaddr_p, sockaddr_len) < 0) {
        int errno_saved = errno;
        LOG_ERROR("Client_raw_connect ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
    this->wrtr = Writer_new(sockfd);
    this->rdr = Reader_new(sockfd);

}
void Client_connect(ClientRef this, char* host, int portno)
{
    int sockfd, n;
    CLIENT_CHECK_TAG(this)
    struct sockaddr_in serv_addr;
    struct hostent *hostent;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        LOG_ERROR("ERROR opening socket");

    hostent = gethostbyname(host);
    if (hostent == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)hostent->h_addr, (char *)&serv_addr.sin_addr.s_addr, hostent->h_length);
    serv_addr.sin_port = htons(portno);
    LOG_FMT("Client_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
    this->wrtr = Writer_new(sockfd);
    this->rdr = Reader_new(sockfd);

}
void Client_roundtrip(ClientRef this, const char* req_buffers[], MessageRef* response_ptr)
{
    CLIENT_CHECK_TAG(this)
    int buf_index = 0;
    int buf_len;
    char* buf;
    while(req_buffers[buf_index] != NULL) {
        buf = (char*)req_buffers[buf_index];
        buf_len = strlen(buf);
        int bytes_written = write(this->sock, buf, buf_len);
        buf_index++;
    }
    int rc = Reader_read(this->rdr, response_ptr);
    if(rc != READER_OK) {
        int errno_saved = errno;
        LOG_ERROR("bad rc from Reader_read rc: errno %d\n", errno_saved);
        assert(false);
    }
}
void Client_request_round_trip(ClientRef this, MessageRef request, MessageRef* response_ptr)
{
    CLIENT_CHECK_TAG(this)
    IOBufferRef req_io_buf = Message_serialize(request);
    Writer_write_chunk(this->wrtr, IOBuffer_data(req_io_buf), IOBuffer_data_len(req_io_buf));

    int rc = Reader_read(this->rdr, response_ptr);
    if(rc != READER_OK) {
        int errno_saved = errno;
        LOG_ERROR("bad rc from Reader_read rc: errno %d\n", errno_saved);
        assert(false);
    }
}
