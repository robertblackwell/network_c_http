
#include <errno.h>
#include <stdlib.h>
#include <rbl/logger.h>
#include "listener_ctx.h"
#define R_STATE_INITIAL 22
#define R_STATE_READY 33
#define R_STATE_EAGAIN 44
#define R_STATE_STOPPED 55
#define R_STATE_ERROR 66
void postable_try_read(RunloopRef rl, void* arg);
void postable_on_read_ready(RunloopRef rl, void* arg);


ReaderCtxRef reader_ctx_new()
{
    ReaderCtxRef ctx = malloc(sizeof(ReaderCtx));
    return ctx;
}
void reader_ctx_init(ReaderCtxRef rdctx, RunloopRef rl, int fd)
{
    RBL_SET_TAG(RDCTX_TAG, rdctx)
    RBL_SET_END_TAG(RDCTX_TAG, rdctx)
    RBL_LOG_FMT("reader_ctx_init fd: %d", fd)
    rdctx->stream = runloop_stream_new(rl, fd);
    rdctx->r_state = R_STATE_INITIAL;
    rdctx->line_buffer_length = 0;
    rdctx->line_buffer_max = 900;
}
void reader_ctx_free(ReaderCtxRef p)
{
    free(p);
}
void reset_line(ReaderCtxRef ctx)
{
    ctx->line_buffer_length = 0;
}
char* parse_line(ReaderCtxRef ctx, char* buffer, int len)
{
    for(int i = 0; i < len; i++){
        char ch = buffer[i];
        if(ch == '\n') {
            ctx->line_buffer[ctx->line_buffer_length] = '\0';
            return (ctx->line_buffer);
        } else {
            int j = ctx->line_buffer_length;
            if(j < ctx->line_buffer_max) {
                ctx->line_buffer[j] = ch;
                ctx->line_buffer_length++;
            } else {
                assert(0);
            }
            return NULL;
        }
    }
    return NULL;
}
void try_read(ReaderCtxRef ctx)
{
    ASSERT_NOT_NULL(ctx);
    RBL_CHECK_TAG(RDCTX_TAG, ctx)
    RBL_CHECK_END_TAG(RDCTX_TAG, ctx)

    RunloopEventRef stream = ctx->stream;
    SOCKW_CHECK_TAG(stream)
    SOCKW_CHECK_END_TAG(stream)
    int fd = runloop_stream_get_fd(stream);
    RBL_LOG_FMT("try_read fd: %d", fd);
    RunloopRef rl = runloop_stream_get_runloop(stream);
    char buffer[200];
    int rn = read(fd, buffer, 100);
    int errno_saved = errno;
    if (rn > 0) {
        buffer[rn] = '\0';
        RBL_LOG_FMT("try_read rn: %d buffer: %s", rn, buffer)
        ctx->r_state = R_STATE_READY;
        char* line = parse_line(ctx, buffer, rn);
        if(line == NULL) {
            runloop_post(rl, postable_try_read, ctx);
        } else {
            RBL_LOG_FMT("try_read got a line %s", line)
        }
    } else if(rn == 0) {
        RBL_LOG_FMT("try_read rn: %d", rn)
        ctx->r_state = R_STATE_ERROR;
    } else {
        if (errno_saved == EAGAIN) {
            ctx->r_state = R_STATE_EAGAIN;
            RBL_LOG_FMT("try_read EAGAIN")
            runloop_stream_arm_read(stream, postable_on_read_ready, ctx);
        } else {
            ctx->r_state = R_STATE_ERROR;
            RBL_LOG_FMT("try_read ERROR %d %s", errno_saved, strerror(errno_saved))
        }
    }
}
void postable_reader_start(RunloopRef rl, void* arg)
{
    ReaderCtxRef ctx = arg;
    ASSERT_NOT_NULL(ctx);
    RBL_CHECK_TAG(RDCTX_TAG, ctx)
    RBL_CHECK_END_TAG(RDCTX_TAG, ctx)
    RunloopEventRef stream = ctx->stream;
    SOCKW_CHECK_TAG(stream)
    SOCKW_CHECK_END_TAG(stream)
    RBL_LOG_FMT("postable_reader_start fd: %d", runloop_stream_get_fd(stream));
    switch(ctx->r_state) {
        case R_STATE_INITIAL:
            ctx->r_state = R_STATE_READY;
            try_read(ctx);
            break;
        case R_STATE_READY:
        case R_STATE_EAGAIN:
        case R_STATE_STOPPED:
        case R_STATE_ERROR:
        default:
            assert(0);
            break;
    }
}
void postable_try_read(RunloopRef rl, void* arg)
{
    ReaderCtxRef ctx = arg;
    ASSERT_NOT_NULL(ctx);
    RBL_CHECK_TAG(RDCTX_TAG, ctx)
    RBL_CHECK_END_TAG(RDCTX_TAG, ctx)
    RunloopEventRef stream = ctx->stream;
    SOCKW_CHECK_TAG(stream)
    SOCKW_CHECK_END_TAG(stream)
    switch(ctx->r_state) {
        case R_STATE_READY:
            try_read(ctx);
            break;
        case R_STATE_EAGAIN:
        case R_STATE_INITIAL:
        case R_STATE_STOPPED:
        case R_STATE_ERROR:
        default:
            assert(0);
            break;
    }
}
void postable_on_read_ready(RunloopRef rl, void* arg)
{
    ReaderCtxRef ctx = arg;
    ASSERT_NOT_NULL(ctx);
    RBL_CHECK_TAG(RDCTX_TAG, ctx)
    RBL_CHECK_END_TAG(RDCTX_TAG, ctx)
    RunloopEventRef stream = ctx->stream;
    SOCKW_CHECK_TAG(stream)
    SOCKW_CHECK_END_TAG(stream)
    switch(ctx->r_state) {
        case R_STATE_EAGAIN:
            ctx->r_state = R_STATE_READY;
        case R_STATE_READY:
            try_read(ctx);
            break;
        case R_STATE_INITIAL:
        case R_STATE_STOPPED:
        case R_STATE_ERROR:
        default:
            assert(0);
            break;
    }

}