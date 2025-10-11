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
static void try_read(TcpStream* ctx);
static void read_clean_termination(TcpStream* ctx);
static void read_error_termination(TcpStream* ctx, int err);
static void postable_reader(RunloopRef rl, void* arg);
static bool is_read_pending(TcpStreamRef ctx);

void tcp_read(TcpStreamRef ctx, IOBufferRef input_buffer, TcpReadCallback read_cb, void* arg)
{
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    assert(! is_read_pending(ctx));
    assert(IOBuffer_space_len(input_buffer) != 0);
    RunloopRef rl = runloop_stream_get_runloop(ctx->rlstream_ref);
    TcpReader* rdctx = &(ctx->reader);
    rdctx->read_cb = read_cb;
    rdctx->read_cb_arg = arg;
    rdctx->input_buffer = input_buffer;
    try_read(ctx);
}
static bool is_read_pending(TcpStreamRef ctx)
{
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    assert(
        ((ctx->reader.input_buffer != NULL)&&(ctx->reader.read_cb != NULL))
        ||
        ((ctx->reader.input_buffer == NULL)&&(ctx->reader.read_cb == NULL))
    );
    return ((ctx->reader.input_buffer != NULL)&&(ctx->reader.read_cb != NULL));

}

void read_ready_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    TcpStream* ctx = (TcpStream*)read_ctx_ref_arg;
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    RunloopStreamRef rlstream = ctx->rlstream_ref ;
    RunloopRef runloop_ref = runloop_stream_get_runloop(rlstream);
    RBL_LOG_FMT("read_ready_callback fd %d read_state: %d\n", ctx->stream_fd, ctx->reader.read_state);
    switch(ctx->reader.read_state){
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
        case RD_STATE_EAGAIN:
            ctx->reader.read_state = RD_STATE_READY;
            try_read(ctx);
            break;
        case RD_STATE_ERROR:
            break;
        case RD_STATE_STOPPED:
            break;
        default:
            assert(false);
    }
}
static void invoke_read_cb(TcpStreamRef ctx, int nread, int errno_value)
{
    TcpReader* rdctx = &(ctx->reader);
    TcpReadCallback* cb = ctx->reader.read_cb;
    assert(cb != NULL);
    void* arg = rdctx->read_cb_arg;
    IOBufferRef iob = rdctx->input_buffer;
    rdctx->input_buffer = NULL;
    rdctx->read_cb = NULL;
    rdctx->read_cb_arg = NULL;
    (*cb)(arg, errno_value);
}
static void try_read(TcpStream* ctx)
{
    RunloopStreamRef rlstream = ctx->rlstream_ref;
    RunloopRef runloop_ref = runloop_stream_get_runloop(rlstream);
    IOBufferRef iob = ctx->reader.input_buffer;
    assert(iob != NULL);
    assert(ctx->stream_fd == runloop_stream_get_fd(rlstream));
    RBL_LOG_FMT("try_read: read_state %d fd: %d \n", ctx->reader.read_state, ctx->stream_fd);
    switch(ctx->reader.read_state) {
        case RD_STATE_EAGAIN:
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
            break;
        case RD_STATE_ERROR:
        case RD_STATE_STOPPED:
            return;
    }
    int fd = runloop_stream_get_fd(rlstream);
    void* buf = IOBuffer_space(iob);
    int len = IOBuffer_space_len(iob);
    int nread = read(fd, buf, len);
    int errno_saved = errno;
    char* s;
    if(nread > 0) {
        ctx->reader.read_state = RD_STATE_READY;
        IOBuffer_commit(iob, nread);
        RBL_LOG_FMT("try_read nread: %d read_state: %d buf:%s\n", nread, ctx->reader.read_state, IOBuffer_cstr(buf));
        invoke_read_cb(ctx, nread, 0);
    } else if (nread == 0) {
        // eof
        invoke_read_cb(ctx, nread, errno_saved);
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_read ERROR EAGAIN %s\n", "");
            ctx->reader.read_state = RD_STATE_EAGAIN;
            runloop_stream_arm_read(rlstream, read_ready_callback, ctx);
        } else {
            RBL_LOG_FMT("try_read ERROR errno: %d description %s\n", errno_saved, strerror(errno_saved));
            invoke_read_cb(ctx, nread, errno_saved);
        }
    }
}
void postable_reader(RunloopRef rl, void* arg)
{
    TcpStream* ctx = arg;
    try_read(ctx);
}
static void read_clean_termination(TcpStream* ctx)
{
    RBL_LOG_FMT("read_clean_termination fd %d read_state: %d\n", ctx->stream_fd, ctx->reader.read_state);
    ctx->reader.read_state = RD_STATE_STOPPED;
    runloop_stream_deregister(ctx->rlstream_ref);
    runloop_stream_disarm_read(ctx->rlstream_ref);
    runloop_stream_free(ctx->rlstream_ref);
}
static void read_error_termination(TcpStream* ctx, int err)
{
    RBL_LOG_FMT("read_error_termination fd %d read_state: %d \n", ctx->stream_fd, ctx->reader.read_state);
    ctx->reader.read_state = RD_STATE_ERROR;
    runloop_stream_deregister(ctx->rlstream_ref);
    runloop_stream_disarm_read(ctx->rlstream_ref);
    runloop_stream_free(ctx->rlstream_ref);
}
#if 0
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    StreamTable* rdr = (StreamTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        TcpStream* ctx = &(rdr->ctx_table[i]);
        RBL_LOG_FMT("read_thread_func loop i: %d fd %d read_state: %d\n", i, ctx->readfd, ctx->read_state);

        rdr->ctx_table[i].reader = runloop_stream_new(runloop_ref, ctx->readfd);
        RunloopStreamRef sw = rdr->ctx_table[i].reader;
        // runloop_stream_register(sw);
        // runloop_stream_arm_read(sw, &read_callback, (void *) ctx);
        runloop_post(runloop_ref, postable_reader, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    int total_chars = 0;
    for(int i = 0; i < rdr->count; i++) {
        TcpStream* ctx = &(rdr->ctx_table[i]);
        total_chars += ctx->read_char_count;
        printf("Reader index: %d char count: %d \n", i, ctx->read_char_count);
    }
    printf("Reader total char count : %d\n", total_chars);

    return NULL;
}
#endif
#if 0
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
#endif