#define ENABLE_LOG

#include <c_http/demo_protocol/demo_client.h>
#include <c_http/common/alloc.h>
#include <c_http/common/cbuffer.h>
#include <c_http/common/message.h>
#include <c_http/demo_protocol/demo_sync_reader.h>
#include <c_http/demo_protocol/demo_sync_writer.h>
#include <c_http/logger.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define DemoClient_TAG "DECLNT"
#include <c_http/check_tag.h>

struct DemoClient_s {
    DECLARE_TAG;
    int       sock;
    DemoSyncWriterRef  wrtr;
    DemoSyncReaderRef  rdr;
};

DemoClientRef democlient_new()
{
    DemoClientRef this = eg_alloc(sizeof(DemoClient));
    SET_TAG(DemoClient_TAG, this)
    this->rdr = NULL;
    this->wrtr = NULL;
}
void democlient_dispose(DemoClientRef* this_ptr)
{
    DemoClientRef this = *this_ptr;
   CHECK_TAG(DemoClient_TAG, this)
    LOG_FMT("democlient_dispose %p  %d\n", this, this->sock);
    if(this->rdr) demosync_reader_dispose(&(this->rdr));
    if(this->wrtr) demosync_writer_dispose(&(this->wrtr));
    close(this->sock);
    eg_free(*this_ptr);
    *this_ptr = NULL;
}
void democlient_raw_connect(DemoClientRef this, int sockfd, struct sockaddr* sockaddr_p, int sockaddr_len)
{
    LOG_FMT("democlient_raw_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,sockaddr_p, sockaddr_len) < 0) {
        int errno_saved = errno;
        LOG_ERROR("democlient_raw_connect ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
    this->wrtr =  demosync_writer_new(sockfd);
    this->rdr = demosync_reader_new(sockfd);

}
void democlient_connect(DemoClientRef this, char* host, int portno)
{
    int sockfd, n;
   CHECK_TAG(DemoClient_TAG, this)
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
    LOG_FMT("democlient_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
    this->wrtr = demosync_writer_new(sockfd);
    this->rdr = demosync_reader_new(sockfd);

}
void democlient_roundtrip(DemoClientRef this, const char* req_buffers[], DemoMessageRef* response_ptr)
{
    CHECK_TAG(DemoClient_TAG, this)
    int buf_index = 0;
    int buf_len;
    char* buf;
    while(req_buffers[buf_index] != NULL) {
        buf = (char*)req_buffers[buf_index];
        buf_len = strlen(buf);
        int bytes_written = write(this->sock, buf, buf_len);
        buf_index++;
    }
    IOBufferRef iobx = IOBuffer_new_with_capacity(120);
    int bytes = read(this->sock, IOBuffer_space(iobx), IOBuffer_space_len(iobx));
    int errno_saved = errno;
    printf("demo_client roundtrip bytes %d errno %d buffer: %s \n", bytes, errno_saved, IOBuffer_cstr(iobx));
//    int rc = demosync_reader_read(this->rdr, response_ptr);
//    IOBufferRef ser = demo_message_serialize(response_ptr);
//    printf("democlient roundtrip %s\n", IOBuffer_cstr(ser));
//    if(rc != READER_OK) {
//        int errno_saved = errno;
//        LOG_ERROR("bad rc from SyncReader_read rc: errno %d\n", errno_saved);
//        assert(false);
//    }
}
void democlient_request_round_trip(DemoClientRef this, DemoMessageRef request, DemoMessageRef* response_ptr)
{
    CHECK_TAG(DemoClient_TAG, this)
    IOBufferRef req_io_buf = demo_message_serialize(request);
    demosync_writer_write_chunk(this->wrtr, IOBuffer_data(req_io_buf), IOBuffer_data_len(req_io_buf));

    int rc = demosync_reader_read(this->rdr, response_ptr);
    if(rc != READER_OK) {
        int errno_saved = errno;
        LOG_ERROR("bad rc from SyncReader_read rc: errno %d\n", errno_saved);
        assert(false);
    }
}
