#include <http_in_c/tmpl_protocol/tmpl_sync_socket.h>
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
#define TmplClient_TAG "DECLNT"
#include <rbl/check_tag.h>

/**
 * A tmpl_syncsocket is an object and set of functions that wrap an fd that represents
 * either a pipe or socket and allows the reading and writing of complete TmplMessage
 * packets.
 */
struct TmplSyncSocket_s {
    RBL_DECLARE_TAG;
    int sock;
    TmplMessageParserRef parser_ref;
    ListRef input_message_list;
    ListRef output_message_list;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* ctx, TmplMessageRef msg)
{
    TmplSyncSocketRef client_ref = ctx;
    assert(msg != NULL);
    List_add_back(client_ref->input_message_list, msg);
}

TmplSyncSocketRef tmpl_syncsocket_new(TmplMessageParserRef parser_ref)
{
    TmplSyncSocketRef this = eg_alloc(sizeof(TmplSyncSocket));
    tmpl_syncsocket_init(this);
    return this;
}
TmplSyncSocketRef tmpl_syncsocket_new_from_fd(int fd)
{
    TmplSyncSocketRef this = eg_alloc(sizeof(TmplSyncSocket));
    tmpl_syncsocket_init(this);
    this->sock = fd;
    return this;
}
void tmpl_syncsocket_init(TmplSyncSocketRef this)
{
    RBL_SET_TAG(TmplClient_TAG, this)
    RBL_SET_END_TAG(TmplClient_TAG, this)
    this->parser_ref = tmpl_message_parser_new(&on_new_message, this);
    this->input_message_list = List_new();
    this->output_message_list = List_new();
}
void tmpl_syncsocket_free(TmplSyncSocketRef this)
{
    RBL_CHECK_TAG(TmplClient_TAG, this)
    RBL_CHECK_END_TAG(TmplClient_TAG, this)
    RBL_LOG_FMT("tmpl_syncsocket_free %p  %d\n", this, this->sock);
    close(this->sock);
    eg_free(this);
}

void tmpl_syncsocket_connect(TmplSyncSocketRef this, char* host, int port)
{
    int sockfd, n;
    RBL_CHECK_TAG(TmplClient_TAG, this)
    RBL_CHECK_END_TAG(TmplClient_TAG, this)
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
    RBL_LOG_FMT("tmpl_syncsocket_connect %p sockfd: %d\n", this, sockfd);
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("ERROR client %p connecting sockfd: % derrno: %d\n", this, sockfd, errno_saved);
    }
    this->sock = sockfd;
}
void tmpl_syncsocket_close(TmplSyncSocketRef sock)
{

}
int tmpl_syncsocket_write_message(TmplSyncSocketRef client_ref, TmplMessageRef msg_ref)
{
    IOBufferRef outbuf = tmpl_message_serialize(msg_ref);
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
int tmpl_syncsocket_read_message(TmplSyncSocketRef client_ref, TmplMessageRef* msg_ref_ptr)
{
    while(1) {
        if(List_size(client_ref->input_message_list) > 0) {
            TmplMessageRef m = List_remove_first(client_ref->input_message_list);
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
#if 1
                tmpl_message_parser_consume(client_ref->parser_ref, iob);
#else
                int rc = TmplMessageParser_consume(client_ref->parser_ref, iob);
                if(rc != 0) {
                    return -1;
                }
#endif
            } else {
                return -1;
            }
        }
    }
}