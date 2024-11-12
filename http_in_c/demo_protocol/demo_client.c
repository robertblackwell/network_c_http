#define CHLOG_ON

#include <http_in_c/demo_protocol/demo_client.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/cbuffer.h>
#include <http_in_c/demo_protocol/demo_message.h>
#include <http_in_c/demo_protocol/demo_parser.h>
#include <rbl/logger.h>
#include <http_in_c/common/list.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <http_in_c/demo_protocol/demo_parser.h>
#define DemoClient_TAG "DECLNT"
#include <rbl/check_tag.h>


struct DemoClient_s {
    RBL_DECLARE_TAG;
    int             sock;
    DemoParserRef   parser_ref;
    ListRef         input_message_list;
    ListRef         output_message_list;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* ctx, DemoMessageRef msg)
{
    DemoClientRef client_ref = ctx;
    assert(msg != NULL);
    List_add_back(client_ref->input_message_list, msg);
}

DemoClientRef democlient_new(DemoParserRef parser_ref)
{
    DemoClientRef this = eg_alloc(sizeof(DemoClient));
    democlient_init(this);
    return this;
}
void democlient_init(DemoClientRef this)
{
    RBL_SET_TAG(DemoClient_TAG, this)
    RBL_SET_END_TAG(DemoClient_TAG, this)
    this->parser_ref = DemoParser_new(&on_new_message, this);
    this->input_message_list = List_new(NULL);
    this->output_message_list = List_new(NULL);
}
void democlient_free(DemoClientRef this)
{
    RBL_CHECK_TAG(DemoClient_TAG, this)
    RBL_CHECK_END_TAG(DemoClient_TAG, this)
    RBL_LOG_FMT("democlient_dispose %p  %d\n", this, this->sock);
    close(this->sock);
    eg_free(this);
}

void democlient_connect(DemoClientRef this, char* host, int portno)
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
    serv_addr.sin_family = AF_INET;
    bcopy((char *)hostent->h_addr, (char *)&serv_addr.sin_addr.s_addr, hostent->h_length);
    serv_addr.sin_port = htons(portno);
    RBL_LOG_FMT("democlient_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
}
int democlient_write_message(DemoClientRef client_ref, DemoMessageRef msg_ref)
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
int democlient_read_message(DemoClientRef client_ref, DemoMessageRef* msg_ref_ptr)
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
            if (bytes_read > 0) {
                IOBuffer_commit(iob, (int) bytes_read);
                RBL_LOG_FMT("response raw: %s \n", IOBuffer_cstr(iob));
                DemoParser_consume(client_ref->parser_ref, iob);
            } else {
                return -1;
            }
        }
    }
}