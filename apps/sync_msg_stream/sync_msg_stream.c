#include "sync_msg_stream.h"
#include <src/common/alloc.h>
#include <src/common/cbuffer.h>
#include <common/iobuffer.h>
#include <rbl/logger.h>
#include <src/common/list.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define SyncMsgStream_TAG "SMSGSRTM"
#include <arpa/inet.h>
#include <rbl/check_tag.h>

/**
 * A sync_msg_stream is an object and set of functions that wrap an fd that represents
 * either a pipe or socket and allows the reading and writing of complete Message
 * packets.
 */
struct SyncMsgStream_s {
    RBL_DECLARE_TAG;
    int sock;
    MSG_PARSER_REF parser_ref;
    ListRef input_message_list;
    ListRef output_message_list;
    RBL_DECLARE_END_TAG;
};
void on_new_message(void* ctx, MSG_REF msg, int error)
{
    SyncMsgStreamRef client_ref = ctx;
    assert(msg != NULL);
    List_add_back(client_ref->input_message_list, msg);
}

SyncMsgStreamRef sync_msg_stream_new(MSG_PARSER_REF parser_ref)
{
    SyncMsgStreamRef this = eg_alloc(sizeof(SyncMsgStream));
    sync_msg_stream_init(this, parser_ref);
    return this;
}
SyncMsgStreamRef sync_msg_stream_new_from_fd(MSG_PARSER_REF parser, int fd)
{
    SyncMsgStreamRef this = eg_alloc(sizeof(SyncMsgStream));
    sync_msg_stream_init(this, parser);
    this->sock = fd;
    return this;
}
void sync_msg_stream_init(SyncMsgStreamRef this, MSG_PARSER_REF parser_ref)
{
    RBL_SET_TAG(SyncMsgStream_TAG, this)
    RBL_SET_END_TAG(SyncMsgStream_TAG, this)
    this->parser_ref = parser_ref;
    this->input_message_list = List_new();
    this->output_message_list = List_new();
}
void sync_msg_stream_free(SyncMsgStreamRef this)
{
    RBL_CHECK_TAG(SyncMsgStream_TAG, this)
    RBL_CHECK_END_TAG(SyncMsgStream_TAG, this)
    RBL_LOG_FMT("sync_msg_stream_free %p  %d", this, this->sock);
    close(this->sock);
    free(this);
}

void sync_msg_stream_connect(SyncMsgStreamRef this, char* host, int port)
{
    int sockfd, n;
    RBL_CHECK_TAG(SyncMsgStream_TAG, this)
    RBL_CHECK_END_TAG(SyncMsgStream_TAG, this)
    struct sockaddr_in serv_addr;
    struct hostent *hostent;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(host);
    RBL_LOG_FMT("sync_msg_stream_connect %p sockfd: %d", this, sockfd);
    int status = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("ERROR client %p port: %d connecting sockfd: % d errno: %d . %s", this, port, sockfd, errno_saved, strerror(errno_saved));
    }
    this->sock = sockfd;
}
void sync_msg_stream_close(SyncMsgStreamRef sock)
{

}
int sync_msg_stream_write(SyncMsgStreamRef client_ref, MSG_REF msg_ref)
{
    IOBufferRef outbuf = MSG_SERIALIZE(msg_ref);
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
int sync_msg_stream_read(SyncMsgStreamRef client_ref, MSG_REF * msg_ref_ptr)
{
    while(1) {
        if(List_size(client_ref->input_message_list) > 0) {
            MSG_REF m = List_remove_first(client_ref->input_message_list);
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
                RBL_LOG_FMT("response raw: %s", IOBuffer_cstr(iob));
                MSG_PARSER_CONSUME(client_ref->parser_ref, iob, &on_new_message, client_ref);
            } else {
                return errno_saved;
            }
        }
    }
}