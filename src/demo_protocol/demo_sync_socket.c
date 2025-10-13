#include <src/demo_protocol/demo_sync_socket.h>
#include <src/common/alloc.h>
#include <src/common/cbuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define DemoClient_TAG "DECLNT"
#include <rbl/check_tag.h>

/**
 * A demo_syncsocket is an object and set of functions that wrap an fd that represents
 * either a pipe or socket and allows the reading and writing of complete DemoMessage
 * packets.
 */
struct DemoSyncSocket_s {
    RBL_DECLARE_TAG;
    int sock;
    DemoMessageParserRef parser_ref;
    ListRef input_message_list;
    ListRef output_message_list;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* ctx, DemoMessageRef msg)
{
    DemoSyncSocketRef client_ref = ctx;
    assert(msg != NULL);
    List_add_back(client_ref->input_message_list, msg);
}

DemoSyncSocketRef demo_syncsocket_new(DemoMessageParserRef parser_ref)
{
    DemoSyncSocketRef this = eg_alloc(sizeof(DemoSyncSocket));
    demo_syncsocket_init(this);
    return this;
}
DemoSyncSocketRef demo_syncsocket_new_from_fd(int fd)
{
    DemoSyncSocketRef this = eg_alloc(sizeof(DemoSyncSocket));
    demo_syncsocket_init(this);
    this->sock = fd;
    return this;
}
void demo_syncsocket_init(DemoSyncSocketRef this)
{
    RBL_SET_TAG(DemoClient_TAG, this)
    RBL_SET_END_TAG(DemoClient_TAG, this)
    this->parser_ref = demo_message_parser_new();
    this->input_message_list = List_new();
    this->output_message_list = List_new();
}
void demo_syncsocket_free(DemoSyncSocketRef this)
{
    RBL_CHECK_TAG(DemoClient_TAG, this)
    RBL_CHECK_END_TAG(DemoClient_TAG, this)
    RBL_LOG_FMT("demo_syncsocket_free %p  %d\n", this, this->sock);
    close(this->sock);
    eg_free(this);
}

void demo_syncsocket_connect(DemoSyncSocketRef this, char* host, int port)
{
    int sockfd, n;
    RBL_CHECK_TAG(DemoClient_TAG, this)
    RBL_CHECK_END_TAG(DemoClient_TAG, this)
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
    RBL_LOG_FMT("demo_syncsocket_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
}
void demo_syncsocket_close(DemoSyncSocketRef sock)
{

}
int demo_syncsocket_write_message(DemoSyncSocketRef client_ref, DemoMessageRef msg_ref)
{
    IOBufferRef outbuf = demo_message_serialize(msg_ref);
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
int demo_syncsocket_read_message(DemoSyncSocketRef client_ref, DemoMessageRef* msg_ref_ptr)
{
    while(1) {
        if(List_size(client_ref->input_message_list) > 0) {
            DemoMessageRef m = List_remove_first(client_ref->input_message_list);
            *msg_ref_ptr = m;
            return 0;
        } else {
            IOBufferRef iob = IOBuffer_new_with_capacity(20000);
            void *buf = IOBuffer_space(iob);
            int len = IOBuffer_space_len(iob);
            long bytes_read = read(client_ref->sock, buf, len);
            int errno_saved = errno;
            if (bytes_read > 0) {
                IOBuffer_commit(iob, (int) bytes_read);
                RBL_LOG_FMT("response raw: %s \n", IOBuffer_cstr(iob));
                demo_message_parser_consume(client_ref->parser_ref, iob, &on_new_message, client_ref);
            } else {
                return errno_saved;
            }
        }
    }
}