#include "stream_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
void async_socket_set_nonblocking(int socket);
static void write_clean_terminate(StreamCtx* ctx);
static void write_error_terminate(StreamCtx* ctx);
static void try_write(StreamCtx* ctx);
static bool is_write_pending(StreamCtxRef ctx);
static void invoke_write_callback(StreamCtxRef ctx, int nwrite, int errnoval);

void async_write(StreamCtxRef ctx, IOBufferRef outbuf, AsyncWriteCallback write_cb, void* arg)
{
    assert(!is_write_pending(ctx));
    assert(write_cb != NULL);
    ctx->writer.output_buffer = outbuf;
    ctx->writer.write_cb = write_cb;
    ctx->writer.write_cb_arg = arg;
}
void postable_try_write(RunloopRef rl, void* arg)
{
    StreamCtxRef ctx = arg;    
    RBL_CHECK_TAG(StreamCtx_TAG, ctx)
    RBL_CHECK_END_TAG(StreamCtx_TAG, ctx)
    try_write(ctx);
}
static bool is_write_pending(StreamCtxRef ctx)
{
    RBL_CHECK_TAG(StreamCtx_TAG, ctx)
    RBL_CHECK_END_TAG(StreamCtx_TAG, ctx)
    assert( 
        ((ctx->writer.output_buffer != NULL) && (ctx->writer.write_cb != NULL))
        || ((ctx->writer.output_buffer == NULL) && (ctx->writer.write_cb == NULL)));
    return (ctx->writer.output_buffer != NULL) ;

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
    StreamCtx* ctx = write_ctx_p_arg;
    RBL_CHECK_TAG(StreamCtx_TAG, ctx)
    RBL_CHECK_END_TAG(StreamCtx_TAG, ctx)
    RunloopStreamRef stream = ctx->stream;
    RunloopRef reactor = runloop_stream_get_runloop(stream);
    runloop_stream_verify(stream);
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
static void try_write(StreamCtx* ctx)
{
    RBL_LOG_FMT("try_write fd: %d write_state: %d\n", ctx->stream_fd, ctx->writer.write_state);

    RunloopRef rl = runloop_stream_get_runloop(ctx->stream);
    void* data = IOBuffer_data(ctx->writer.output_buffer);
    size_t len = IOBuffer_data_len(ctx->writer.output_buffer);
    int nwrite = write(ctx->stream_fd, data, len);
    int errno_saved = errno;
    if (nwrite > 0) {
        RBL_LOG_FMT("try_write DONE fd: %d nwrite: %d \n", ctx->stream_fd, nwrite);
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
            RBL_LOG_FMT("try_write EAGAIN fd: %d write_state: %d\n", ctx->stream_fd, ctx->writer.write_state);
            ctx->writer.write_state = WRT_STATE_EAGAIN;
        } else {
            RBL_LOG_FMT("try_write fd: %d errno %d desc: %s \n", ctx->stream_fd, errno_saved, strerror(errno_saved));
            ctx->writer.write_state = WRT_STATE_ERROR;
            invoke_write_callback(ctx, nwrite, errno_saved);
        }
    }    
}
static void invoke_write_callback(StreamCtxRef ctx, int nwrite, int errnoval)
{
    IOBuffer_free(ctx->writer.output_buffer);
    AsyncReadCallback cb = ctx->writer.write_cb;
    ctx->writer.write_cb = NULL;
    void* arg = ctx->writer.write_cb_arg;
    ctx->writer.write_cb_arg = NULL;
    cb(arg, nwrite, errnoval);
}
static void write_clean_terminate(StreamCtx* ctx)
{   
    ctx->writer.write_state = WRT_STATE_STOPPED;
    
    runloop_stream_disarm_write(ctx->stream);
    runloop_stream_deregister(ctx->stream);
    runloop_stream_free(ctx->stream);
}
static void write_error_terminate(StreamCtx* ctx)
{
    runloop_stream_disarm_write(ctx->stream);
    runloop_stream_deregister(ctx->stream);
    runloop_stream_free(ctx->stream);
}
#if 0
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        StreamCtx* ctx = &(wrtr->ctx_table[i]);
        ctx->writer.write_state = WRT_STATE_INITIAL;
        wrtr->ctx_table[i].stream = runloop_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].timer_ref = runloop_timer_new(runloop_ref);
        // this timer will periodically write a message and when enough writes have happened close down the writer
        runloop_timer_register(wrtr->ctx_table[i].timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);

        RBL_CHECK_TAG(StreamCtx_ATG, ctx)
        RBL_CHECK_END_TAG(StreamCtx_ATG, ctx)
        runloop_timer_checktag(ctx->timer_ref);
        runloop_stream_checktag(ctx->stream);

        runloop_stream_register(ctx->stream);
        runloop_stream_arm_write(ctx->stream, &wrtr_cb, (void *) ctx);
    }
    runloop_run(runloop_ref, 10000000);
    int total_chars = 0;
    for(int i = 0; i < wrtr->count; i++) {
        StreamCtx* ctx = &(wrtr->ctx_table[i]);
        total_chars += ctx->write_char_count;
        printf("Writer index: %d char count: %d \n", i, ctx->write_char_count);
    }
    printf("Writer total char count : %d\n", total_chars);
    return NULL;
}
#endif