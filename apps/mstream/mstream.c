#include "mstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <rbl/check_tag.h>
#include <common/list.h>

#include <common/socket_functions.h>
#define SyncMsgStream_TAG "SMSGSRTM"

struct MStream_s
{
    RBL_DECLARE_TAG;
    const char* host;
    int port;
    int fd;
    ListRef input_msg_list;
    RBL_DECLARE_END_TAG;
};
void new_msg_cb(void* arg, MSG_REF nm, int error_code)
{
    MStreamRef mstream = arg;
    RBL_SET_TAG(SyncMsgStream_TAG, mstream)
    RBL_SET_END_TAG(SyncMsgStream_TAG, mstream)
    assert(nm != NULL);
    List_add_back(mstream->input_msg_list, nm);
}
void mstream_init(MStreamRef stream, const char* host, int port)
{
    assert(stream != NULL);
    RBL_SET_TAG(SyncMsgStream_TAG, stream)
    RBL_SET_END_TAG(SyncMsgStream_TAG, stream)
    stream->host = host;
    stream->port = port;
    stream->fd = create_and_connect_socket(stream->host, stream->port);
    stream->input_msg_list = List_new();
}
MStreamRef mstream_new(const char* host, int port)
{
    MStreamRef stream = malloc(sizeof(struct MStream_s));
    mstream_init(stream, host, port);
    return stream;
}
void mstream_write(MStreamRef stream, MSG_REF msg_ref)
{
    RBL_CHECK_TAG(SyncMsgStream_TAG, stream)
    RBL_CHECK_END_TAG(SyncMsgStream_TAG, stream)
    IOBufferRef send_iob = MSG_SERIALIZE(msg_ref);
    int out_len = IOBuffer_data_len(send_iob);
    ssize_t nw = write(stream->fd, IOBuffer_data(send_iob), IOBuffer_data_len(send_iob));
    int errno_saved = errno;
    if (nw != out_len) {
        if (nw > 0) {
            assert(0);
        } else if (nw == 0) {
            assert(0); // have to start worrying about signals and EAGAIN
        } else {
            assert(0);
        }
    }
}
MSG_REF mstream_read(MStreamRef stream, MSG_PARSER_REF parser)
{
    RBL_CHECK_TAG(SyncMsgStream_TAG, stream)
    RBL_CHECK_END_TAG(SyncMsgStream_TAG, stream)
    while (1) {
        if (List_size(stream->input_msg_list) > 0) {
            MSG_REF m = List_remove_first(stream->input_msg_list);
            return m;
        } else {
            IOBufferRef iob = IOBuffer_new(256);
            size_t rn = read(stream->fd, IOBuffer_space(iob), IOBuffer_space_len(iob));
            int errno_saved = errno;
            if (rn <= 0) {
                assert(rn > 0);
            } else {
                IOBuffer_commit(iob, (int)rn);
                MSG_PARSER_CONSUME(parser, iob, new_msg_cb, stream);
            }
        }
    }
}
void mstream_free(MStreamRef s)
{
    close(s->fd);
    ListIterator itr = List_iterator(s->input_msg_list);
    while (itr != NULL) {
        MSG_REF m = List_itr_unpack(s->input_msg_list, itr);
        MSG_FREE(m);
        itr = List_itr_next(s->input_msg_list, itr);
    }
    free(s);
}

