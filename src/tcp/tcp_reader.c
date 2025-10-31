#include "tcp_stream.h"
#include "tcp_stream_internal.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <src/common/utils.h>

void async_socket_set_nonblocking(int socket);
static void try_read(TcpStream* tcp_stream_ref);
static void postable_reader(RunloopRef rl, void* arg);
static bool is_read_pending(TcpStreamRef tcp_stream_ref);

void tcp_read(TcpStreamRef ctx, IOBufferRef input_buffer, TcpReadCallback read_cb, void* arg)
{
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    assert(input_buffer != NULL);
    assert(! is_read_pending(ctx));
    assert(IOBuffer_space_len(input_buffer) != 0);
    RunloopRef rl = runloop_stream_get_runloop(ctx->rlstream_ref);
    ctx->read_cb = read_cb;
    ctx->read_cb_arg = arg;
    ctx->input_buffer = input_buffer;
    try_read(ctx);
}
static bool is_read_pending(TcpStreamRef tcp_stream_ref)
{
    RBL_CHECK_TAG(TcpStream_TAG, tcp_stream_ref)
    RBL_CHECK_END_TAG(TcpStream_TAG, tcp_stream_ref)
    assert(
        ((tcp_stream_ref->input_buffer != NULL)&&(tcp_stream_ref->read_cb != NULL))
        ||
        ((tcp_stream_ref->input_buffer == NULL)&&(tcp_stream_ref->read_cb == NULL))
    );
    return ((tcp_stream_ref->input_buffer != NULL)&&(tcp_stream_ref->read_cb != NULL));

}

void read_ready_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    TcpStreamRef tcp_stream_ref = (TcpStream*)read_ctx_ref_arg;
    RBL_CHECK_TAG(TcpStream_TAG, tcp_stream_ref)
    RBL_CHECK_END_TAG(TcpStream_TAG, tcp_stream_ref)
    RunloopStreamRef rlstream = tcp_stream_ref->rlstream_ref ;
    RunloopRef runloop_ref = runloop_stream_get_runloop(rlstream);
    RBL_LOG_FMT("read_ready_callback fd %d read_state: %d\n", tcp_stream_ref->stream_fd, tcp_stream_ref->read_state);
    switch(tcp_stream_ref->read_state){
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
        case RD_STATE_EAGAIN:
            tcp_stream_ref->read_state = RD_STATE_READY;
            try_read(tcp_stream_ref);
            break;
        case RD_STATE_ERROR:
            break;
        case RD_STATE_STOPPED:
            break;
        default:
            assert(false);
    }
}
static void invoke_read_cb(TcpStreamRef tcp_stream_ref, int nread, int errno_value)
{
    TcpReadCallback* cb = tcp_stream_ref->read_cb;
    assert(cb != NULL);
    void* arg = tcp_stream_ref->read_cb_arg;
    IOBufferRef iob = tcp_stream_ref->input_buffer;
    tcp_stream_ref->input_buffer = NULL;
    tcp_stream_ref->read_cb = NULL;
    tcp_stream_ref->read_cb_arg = NULL;
    (*cb)(arg, errno_value);
}
static void try_read(TcpStreamRef tcp_stream_ref)
{
    RunloopStreamRef rlstream = tcp_stream_ref->rlstream_ref;
    RunloopRef runloop_ref = runloop_stream_get_runloop(rlstream);
    IOBufferRef iob = tcp_stream_ref->input_buffer;
    assert(iob != NULL);
    assert(tcp_stream_ref->stream_fd == runloop_stream_get_fd(rlstream));
    RBL_LOG_FMT("try_read: read_state %d fd: %d \n", tcp_stream_ref->read_state, tcp_stream_ref->stream_fd);
    switch(tcp_stream_ref->read_state) {
        case RD_STATE_EAGAIN:
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
            break;
        case RD_STATE_ERROR:
        case RD_STATE_STOPPED:
            return;
        default:
            assert(false);
    }
    int fd = runloop_stream_get_fd(rlstream);
    void* buf = IOBuffer_space(iob);
    int len = IOBuffer_space_len(iob);
    int nread = (int)read(fd, buf, len);
    int errno_saved = errno;
    char* s;
    if(nread > 0) {
        tcp_stream_ref->read_state = RD_STATE_READY;
        IOBuffer_commit(iob, nread);
        RBL_LOG_FMT("try_read nread: %d read_state: %d buf:%s\n", nread, tcp_stream_ref->read_state, IOBuffer_cstr(iob));
        invoke_read_cb(tcp_stream_ref, nread, 0);
    } else if (nread == 0) {
        // eof
        invoke_read_cb(tcp_stream_ref, nread, errno_saved);
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_read ERROR EAGAIN %s\n", "");
            tcp_stream_ref->read_state = RD_STATE_EAGAIN;
            runloop_stream_arm_read(rlstream, read_ready_callback, tcp_stream_ref);
        } else {
            RBL_LOG_FMT("try_read ERROR errno: %d description %s\n", errno_saved, strerror(errno_saved));
            invoke_read_cb(tcp_stream_ref, nread, errno_saved);
        }
    }
}
void postable_reader(RunloopRef rl, void* arg)
{
    TcpStreamRef tcp_stream_ref = arg;
    try_read(tcp_stream_ref);
}
