#include "tcp_stream.h"
#include "tcp_stream_internal.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <src/common/utils.h>


void async_socket_set_nonblocking(int socket);
static void write_clean_terminate(TcpStream* ctx);
static void write_error_terminate(TcpStream* ctx);
static void try_write(TcpStream* ctx);
static bool is_write_pending(TcpStreamRef tcp_stream_ref);
static void invoke_write_callback(TcpStreamRef ctx, int nwrite, int errnoval);

void tcp_write(TcpStreamRef tcp_stream_ref, IOBufferRef outbuf, TcpWriteCallback write_cb, void* arg)
{
    assert(!is_write_pending(tcp_stream_ref));
    assert(write_cb != NULL);
    tcp_stream_ref->output_buffer = outbuf;
    tcp_stream_ref->write_cb = write_cb;
    tcp_stream_ref->write_cb_arg = arg;
    try_write(tcp_stream_ref);
}
void postable_try_write(RunloopRef rl, void* arg)
{
    TcpStreamRef ctx = arg;    
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    try_write(ctx);
}
static bool is_write_pending(TcpStreamRef tcp_stream_ref)
{
    RBL_CHECK_TAG(TcpStream_TAG, tcp_stream_ref)
    RBL_CHECK_END_TAG(TcpStream_TAG, tcp_stream_ref)
    assert( 
        ((tcp_stream_ref->output_buffer != NULL) && (tcp_stream_ref->write_cb != NULL))
        || ((tcp_stream_ref->output_buffer == NULL) && (tcp_stream_ref->write_cb == NULL)));
    return (tcp_stream_ref->output_buffer != NULL) ;

}
static void fill_buffer(char* line, char* buffer, int max_len, int required_data_length)
{
    memset(buffer, '?', max_len);
    size_t line_length = strlen(line);
    char* stopping = &(buffer[required_data_length]);
    char* p = buffer;
    while(true) {
        size_t x = sprintf(p, "%s", line);
        p = p + x;
        if((p - buffer) > required_data_length) {
            *p = (char)0;
            break;
        }
    }
    printf("done");
}
// 
static void wrtr_cb(RunloopRef rl, void* write_ctx_p_arg)
{
    TcpStream* ctx = write_ctx_p_arg;
    RBL_CHECK_TAG(TcpStream_TAG, ctx)
    RBL_CHECK_END_TAG(TcpStream_TAG, ctx)
    RunloopStreamRef rlstream = ctx->rlstream_ref;
    RunloopRef reactor = runloop_stream_get_runloop(rlstream);
    runloop_stream_verify(rlstream);
    switch(ctx->write_state) {
        case WRT_STATE_EAGAIN: 
            ctx->write_state = WRT_STATE_READY;
            try_write(ctx);
            break;
        case WRT_STATE_INITIAL:
            // first time
        case WRT_STATE_READY:
            break;
        case WRT_STATE_ERROR:
            break;
        case WRT_STATE_STOPPED:
            break;
        default:
            assert(false);
    } 
}
static void try_write(TcpStream* ctx)
{
    RBL_LOG_FMT("try_write fd: %d write_state: %d\n", ctx->stream_fd, ctx->write_state);

    RunloopRef rl = runloop_stream_get_runloop(ctx->rlstream_ref);
    void* data = IOBuffer_data(ctx->output_buffer);
    size_t len = IOBuffer_data_len(ctx->output_buffer);
    int nwrite = (int)write(ctx->stream_fd, data, len);
    int errno_saved = errno;
    if (nwrite > 0) {
        RBL_LOG_FMT("try_write DONE fd: %d nwrite: %d \n", ctx->stream_fd, nwrite);
        IOBuffer_consume(ctx->output_buffer, nwrite);
        ctx->write_state = WRT_STATE_READY;
        if(IOBuffer_data_len(ctx->output_buffer) == 0) {
            invoke_write_callback(ctx, nwrite, 0);
        } else {
            runloop_post(rl, postable_try_write, ctx);
        }
    } else if(nwrite == 0){
        invoke_write_callback(ctx, nwrite, errno_saved);
    } else {
        if (errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_write EAGAIN fd: %d write_state: %d\n", ctx->stream_fd, ctx->write_state);
            ctx->write_state = WRT_STATE_EAGAIN;
        } else {
            RBL_LOG_FMT("try_write fd: %d errno %d desc: %s \n", ctx->stream_fd, errno_saved, strerror(errno_saved));
            ctx->write_state = WRT_STATE_ERROR;
            invoke_write_callback(ctx, nwrite, errno_saved);
        }
    }    
}
static void invoke_write_callback(TcpStreamRef ctx, int nwrite, int errnoval)
{
    if(ctx->output_buffer) IOBuffer_free(ctx->output_buffer);
    ctx->output_buffer = NULL;
    TcpWriteCallback* cb = ctx->write_cb;
    ctx->write_cb = NULL;
    void* arg = ctx->write_cb_arg;
    ctx->write_cb_arg = NULL;
    cb(arg, errnoval);
}
static void write_clean_terminate(TcpStream* ctx)
{   
    ctx->write_state = WRT_STATE_STOPPED;
    
    runloop_stream_disarm_write(ctx->rlstream_ref);
    runloop_stream_deregister(ctx->rlstream_ref);
    runloop_stream_free(ctx->rlstream_ref);
}
static void write_error_terminate(TcpStream* ctx)
{
    runloop_stream_disarm_write(ctx->rlstream_ref);
    runloop_stream_deregister(ctx->rlstream_ref);
    runloop_stream_free(ctx->rlstream_ref);
}
