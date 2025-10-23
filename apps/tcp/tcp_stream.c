#include "tcp_stream_internal.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <src/common/utils.h>
#include "tcp_stream.h"
#include "tcp_stream_internal.h"

void async_socket_set_nonblocking(int socket);

TcpStreamRef tcp_stream_new(RunloopRef rl, int fd)
{
    TcpStreamRef tcp_stream_ref = malloc(sizeof(TcpStream));
    tcp_stream_init(tcp_stream_ref, rl, fd);
    return tcp_stream_ref;
}
void tcp_stream_init(TcpStreamRef tcp_stream_ref, RunloopRef rl, int fd)
{
    RBL_SET_TAG(TcpStream_TAG, tcp_stream_ref)
    RBL_SET_END_TAG(TcpStream_TAG, tcp_stream_ref)
    tcp_stream_ref->rlstream_ref = runloop_stream_new(rl, fd);
    tcp_stream_ref->reader.read_state = RD_STATE_INITIAL;
    tcp_stream_ref->writer.write_state = WRT_STATE_INITIAL;
    tcp_stream_ref->stream_fd = runloop_stream_get_fd(tcp_stream_ref->rlstream_ref);
}
void tcp_stream_deinit(TcpStreamRef t)
{
    RBL_CHECK_TAG(TcpStream_TAG, t)
    RBL_CHECK_END_TAG(TcpStream_TAG, t)
    assert(0);
    // runloop_stream_deinit(t->rlstream_ref);
}
void tcp_stream_free(TcpStreamRef t)
{
    RBL_CHECK_TAG(TcpStream_TAG, t)
    RBL_CHECK_END_TAG(TcpStream_TAG, t)
    runloop_stream_free(t->rlstream_ref);
}
RunloopRef tcp_stream_get_runloop(TcpStreamRef tcp_stream_ref)
{
    return runloop_stream_get_runloop(tcp_stream_ref->rlstream_ref);
}
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, modFlags2);
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set non blocking error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    
    RBL_ASSERT((result == 0), "set socket non blocking");
}
