#include <http_in_c/http_protocol/http_sync_socket.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/cbuffer.h>
#include <rbl/logger.h>
#include <http_in_c/common/list.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <http_in_c/http_protocol/http_message_parser.h>
#define HttpSyncSocket_TAG "HTTPSYNSK"
#include <rbl/check_tag.h>

/**
 * A http_syncsocket is an object and set of functions that wrap an fd that represents
 * either a pipe or socket and allows the reading and writing of complete HttpMessage
 * packets.
 */
struct HttpSyncSocket_s {
    RBL_DECLARE_TAG;
    int sock;
    HttpMessageParserRef parser_ref;
    ListRef input_message_list;
    ListRef output_message_list;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* client_ref_ctx, HttpMessageRef msg)
{
//    HttpMessageParserRef pref = ctx;
    HttpSyncSocketRef client_ref = client_ref_ctx;
    assert(msg != NULL);
    List_add_back(client_ref->input_message_list, msg);
}

HttpSyncSocketRef http_syncsocket_new(HttpMessageParserRef parser_ref)
{
    HttpSyncSocketRef this = eg_alloc(sizeof(HttpSyncSocket));
    http_syncsocket_init(this);
    return this;
}
HttpSyncSocketRef http_syncsocket_new_from_fd(int fd)
{
    HttpSyncSocketRef this = eg_alloc(sizeof(HttpSyncSocket));
    http_syncsocket_init(this);
    this->sock = fd;
    return this;
}
void http_syncsocket_init(HttpSyncSocketRef this)
{
    RBL_SET_TAG(HttpSyncSocket_TAG, this)
    RBL_SET_END_TAG(HttpSyncSocket_TAG, this)
    this->parser_ref = http_message_parser_new(&on_new_message, this);
    this->input_message_list = List_new();
    this->output_message_list = List_new();
}
void http_syncsocket_free(HttpSyncSocketRef this)
{
    RBL_CHECK_TAG(HttpSyncSocket_TAG, this)
    RBL_CHECK_END_TAG(HttpSyncSocket_TAG, this)
    RBL_LOG_FMT("http_syncsocket_free %p  %d\n", this, this->sock);
    close(this->sock);
    eg_free(this);
}

void http_syncsocket_connect(HttpSyncSocketRef this, char* host, int port)
{
    int sockfd, n;
    RBL_CHECK_TAG(HttpSyncSocket_TAG, this)
    RBL_CHECK_END_TAG(HttpSyncSocket_TAG, this)
    struct sockaddr_in serv_addr;
    struct hostent *hostent;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        RBL_LOG_ERROR("ERROR opening socket");

    hostent = gethostbyname(host);
    if (hostent == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // this method is deprecated -
    serv_addr.sin_family = AF_INET;
    // bcopy((char *)hostent->h_addr, (char *)&serv_addr.sin_addr.s_addr, hostent->h_length);
    bcopy((char *)hostent->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, hostent->h_length);
    serv_addr.sin_port = htons(port);
    RBL_LOG_FMT("http_syncsocket_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
}
void http_syncsocket_close(HttpSyncSocketRef sock)
{

}
int http_syncsocket_write_message(HttpSyncSocketRef client_ref, HttpMessageRef msg_ref)
{
    IOBufferRef outbuf = http_message_serialize(msg_ref);
    void* out_data = IOBuffer_data(outbuf);
    int out_len = IOBuffer_data_len(outbuf);
    assert(out_len > 0);
    long bytes_written = write(client_ref->sock, out_data, out_len);
    int errno_saved = errno;
    if(bytes_written != out_len) {
        if (bytes_written > 0) {
            assert(0);
        } else if (bytes_written == 0) {
            return -1;
        }
    }
    return errno_saved;
}
int http_syncsocket_read_message(HttpSyncSocketRef client_ref, HttpMessageRef* msg_ref_ptr)
{
    while(1) {
        if(List_size(client_ref->input_message_list) > 0) {
            HttpMessageRef m = List_remove_first(client_ref->input_message_list);
            *msg_ref_ptr = m;
            return 0;
        } else {
            IOBufferRef iob = IOBuffer_new_with_capacity(20000);
            void *buf = IOBuffer_space(iob);
            int len = IOBuffer_space_len(iob);
            long bytes_read = read(client_ref->sock, buf, len);
            if (bytes_read > 0) {
                IOBuffer_commit(iob, (int) bytes_read);
                RBL_LOG_FMT("response raw: %s \n", IOBuffer_cstr(iob));
                int rc = http_message_parser_consume_buffer(client_ref->parser_ref, iob);
                int x = llhttp_message_needs_eof(client_ref->parser_ref->m_llhttp_ptr);
                if(rc != 0) {
                    return -1;
                }
            } else {
                return -1;
            }
        }
    }
}