
#include <pthread.h>
#ifdef APPLE_FLAG
#include <sys/_pthread/_pthread_t.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <rbl/unittest.h>
#include "sync_msg_stream.h"

typedef struct SyncMsgStream_s {
    int fd;
    NewlineMsgParserRef parser_ref;
    NewlineMsgRef new_msg;
} SyncMsgStream, *SyncMsgStreamRef;

void new_msg_callback(void* arg, NewlineMsgRef new_msg, int error)
{
    SyncMsgStreamRef sstream = arg;
    assert(sstream->new_msg == NULL);
    sstream->new_msg = new_msg;
}
SyncMsgStreamRef sync_msg_stream_new()
{
    SyncMsgStreamRef s = malloc(sizeof(SyncMsgStream));
    s->fd = -1;
    s->parser_ref = newline_msg_parser_new(new_msg_callback, s);
    return s;
}
void sync_msg_stream_free(SyncMsgStreamRef s)
{
    newline_msg_parser_free(s->parser_ref);
    if(s->fd != -1) close(s->fd);
    free(s);
}

void sync_msg_stream_connect(SyncMsgStreamRef sync_stream, int port)
{
    struct sockaddr_in server;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    const int status = connect(lfd, (struct sockaddr *) &server, sizeof server);
    assert(status == 0);
    sync_stream->fd = lfd;
}
void sync_msg_stream_send(SyncMsgStreamRef sync_stream, NewlineMsgRef msgref)
{
    IOBufferRef iob = newline_msg_serialize(msgref);
    ssize_t len = send(sync_stream->fd, IOBuffer_data(iob), IOBuffer_data_len(iob), 0);
    assert(len == IOBuffer_data_len(iob));
}
NewlineMsgRef sync_msg_stream_recv(SyncMsgStreamRef sync_stream)
{
    IOBufferRef iob = IOBuffer_new_with_capacity(1024);
    while(1) {
        IOBuffer_reset(iob);
        ssize_t len = recv(sync_stream->fd, IOBuffer_space(iob), IOBuffer_space_len(iob), 0);
        assert(len > 0);
        IOBuffer_commit(iob, (int)len);
        newline_msg_parser_consume(sync_stream->parser_ref, iob);
        assert(IOBuffer_data_len(iob) == 0);
        if (sync_stream->new_msg != NULL) {
            NewlineMsgRef tmp = sync_stream->new_msg;
            sync_stream->new_msg = NULL;
            IOBuffer_free(iob);
            return tmp;
        }
    }
}
