#include "mstream.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
struct MStream_s
{
    const char* host;
    int port;
    int fd;
    MSG_REF new_msg;
};
void new_msg_cb(void* arg, MSG_REF nm, int error_code)
{
    MStreamRef mstream = arg;
    mstream->new_msg = nm;
}
static int local_connect(const char* host, int port)
{
    struct sockaddr_in server;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = inet_addr(host);
    const int status = connect(lfd, (struct sockaddr *)&server, sizeof server);
    assert(status == 0);
    return lfd;
}
void mstream_init(MStreamRef stream, const char* host, int port)
{
    assert(stream != NULL);
    stream->host = host;
    stream->port = port;
    stream->fd = local_connect(stream->host, stream->port);
    stream->new_msg = NULL;
}
MStreamRef mstream_new(const char* host, int port)
{
    MStreamRef stream = malloc(sizeof(struct MStream_s));
    mstream_init(stream, host, port);
    return stream;
}
void mstream_write(MStreamRef stream, MSG_REF msg_ref)
{
    IOBufferRef send_iob = MSG_SERIALIZE(msg_ref);
    size_t nw = write(stream->fd, IOBuffer_data(send_iob), IOBuffer_data_len(send_iob));
    assert(nw > 0);
}
MSG_REF mstream_read(MStreamRef stream, MSG_PARSER_REF parser)
{
    IOBufferRef iob_response = IOBuffer_new(256);
    size_t rn = read(stream->fd, IOBuffer_space(iob_response), IOBuffer_space_len(iob_response));
    assert(rn > 0);
    IOBuffer_commit(iob_response, (int)rn);
    MSG_PARSER_CONSUME(parser, iob_response, new_msg_cb, stream);
    if (stream->new_msg == NULL)
        assert(stream->new_msg != NULL);
    IOBuffer_free(iob_response);
    iob_response = NULL;
    MSG_REF response_msg = stream->new_msg;
    stream->new_msg = NULL;
    return response_msg;
}
void mstream_free(MStreamRef s)
{
    close(s->fd);
    free(s);
}

