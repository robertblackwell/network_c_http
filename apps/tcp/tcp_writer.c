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
    tcp_stream_ref->writer.output_buffer = outbuf;
    tcp_stream_ref->writer.write_cb = write_cb;
    tcp_stream_ref->writer.write_cb_arg = arg;
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
        ((tcp_stream_ref->writer.output_buffer != NULL) && (tcp_stream_ref->writer.write_cb != NULL))
        || ((tcp_stream_ref->writer.output_buffer == NULL) && (tcp_stream_ref->writer.write_cb == NULL)));
    return (tcp_stream_ref->writer.output_buffer != NULL) ;

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
    switch(ctx->writer.write_state) {
        case WRT_STATE_EAGAIN: 
            ctx->writer.write_state = WRT_STATE_READY;
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
    RBL_LOG_FMT("try_write fd: %d write_state: %d", ctx->stream_fd, ctx->writer.write_state);

    RunloopRef rl = runloop_stream_get_runloop(ctx->rlstream_ref);
    void* data = IOBuffer_data(ctx->writer.output_buffer);
    size_t len = IOBuffer_data_len(ctx->writer.output_buffer);
    int nwrite = (int)write(ctx->stream_fd, data, len);
    int errno_saved = errno;
    if (nwrite > 0) {
        RBL_LOG_FMT("try_write DONE fd: %d nwrite: %d", ctx->stream_fd, nwrite);
        IOBuffer_consume(ctx->writer.output_buffer, nwrite);
        ctx->writer.write_state = WRT_STATE_READY;
        if(IOBuffer_data_len(ctx->writer.output_buffer) == 0) {
            invoke_write_callback(ctx, nwrite, 0);
        } else {
            runloop_post(rl, postable_try_write, ctx);
        }
    } else if(nwrite == 0){
        invoke_write_callback(ctx, nwrite, errno_saved);
    } else {
        if (errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_write EAGAIN fd: %d write_state: %d", ctx->stream_fd, ctx->writer.write_state);
            ctx->writer.write_state = WRT_STATE_EAGAIN;
        } else {
            RBL_LOG_FMT("try_write fd: %d errno %d desc: %s ", ctx->stream_fd, errno_saved, strerror(errno_saved));
            ctx->writer.write_state = WRT_STATE_ERROR;
            invoke_write_callback(ctx, nwrite, errno_saved);
        }
    }    
}
static void invoke_write_callback(TcpStreamRef ctx, int nwrite, int errnoval)
{
    ctx->writer.output_buffer = NULL;
    TcpWriteCallback* cb = ctx->writer.write_cb;
    ctx->writer.write_cb = NULL;
    void* arg = ctx->writer.write_cb_arg;
    ctx->writer.write_cb_arg = NULL;
    cb(arg, errnoval);
}
static void write_clean_terminate(TcpStream* ctx)
{   
    ctx->writer.write_state = WRT_STATE_STOPPED;
    
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
#if 0
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        TcpStream* ctx = &(wrtr->ctx_table[i]);
        ctx->writer.write_state = WRT_STATE_INITIAL;
        wrtr->ctx_table[i].stream = runloop_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].timer_ref = runloop_timer_new(runloop_ref);
        // this timer will periodically write a message and when enough writes have happened close down the writer
        runloop_timer_register(wrtr->ctx_table[i].timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);

        RBL_CHECK_TAG(TcpStream_ATG, ctx)
        RBL_CHECK_END_TAG(TcpStream_ATG, ctx)
        runloop_timer_checktag(ctx->timer_ref);
        runloop_stream_checktag(ctx->stream);

        runloop_stream_register(ctx->stream);
        runloop_stream_arm_write(ctx->stream, &wrtr_cb, (void *) ctx);
    }
    runloop_run(runloop_ref, 10000000);
    int total_chars = 0;
    for(int i = 0; i < wrtr->count; i++) {
        TcpStream* ctx = &(wrtr->ctx_table[i]);
        total_chars += ctx->write_char_count;
        printf("Writer index: %d char count: %d \n", i, ctx->write_char_count);
    }
    printf("Writer total char count : %d\n", total_chars);
    return NULL;
}
#endif