
//#define RBL_LOG_ENABLED
//#define RBL_LOG_ALLOW_GLOBAL
#include "io_write_asio.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c//runloop/rl_internal.h>
void async_socket_set_nonblocking(int socket);

/**
 * The writer does the following
 *
 *  wait on a timer to expire, then disarm the timer
 *  wait for the fd to become writeable and then write a show message in blocking mode
 *
 *  disarm writeable events
 *  rearm the timer
 *
 */
void WriteCtx_init(WriteCtx* this, int fd, int myindex, int max, int interval_ms)
{
    RBL_SET_TAG(WriteCtx_ATG, this)
    RBL_SET_END_TAG(WriteCtx_ATG, this)
    this->id = "WRITE";
    this->writefd = fd;
    this->timer_ref = NULL;
    this->asiostream_ref = NULL;
    this->write_count = 0;
    this->writer_index = myindex;
    this->interval_ms = interval_ms;
    this->max_write_count = max;
    this->outbuffer = malloc(20000);
    this->outbuffer_max_length = 20000;
    this->outbuffer_length = 0;
    async_socket_set_nonblocking(fd);
}
void WriteCtx_set_stream_ref(WriteCtx* ctx, RunloopRef rl, int fd)
{
#if 0
    ctx->stream_ref = runloop_stream_new(rl, fd);
    ctx->asiostream_ref = NULL;
#else
    ctx->asiostream_ref = asio_stream_new(rl, fd);
    ctx->stream_ref = ctx->asiostream_ref->runloop_stream_ref;
#endif
}


void WriterTable_init(WriterTable* this)
{
    RBL_SET_TAG(WriterTable_TAG, this)
    RBL_SET_END_TAG(WriterTable_TAG, this)
    this->count = 0;
}
WriterTable* WriterTable_new()
{
    WriterTable* tmp = malloc(sizeof(WriterTable));
    WriterTable_init(tmp);
    return tmp;
}
void WriterTable_free(WriterTable* this)
{
    RBL_CHECK_TAG(WriterTable_TAG, this)
    RBL_CHECK_END_TAG(WriterTable_TAG, this)
    free(this);
}
void WriterTable_add_fd(WriterTable* this, int fd, int max, int interval_ms)
{
    RBL_CHECK_TAG(WriterTable_TAG, this)
    RBL_CHECK_END_TAG(WriterTable_TAG, this)
    async_socket_set_nonblocking(fd);

    WriteCtx* ctx = &(this->ctx_table[this->count]);
    WriteCtx_init(ctx, fd, this->count, max, interval_ms);
    this->count++;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* arg);

static size_t fill_buffer(char* line, char* buffer, int max_len, int required_data_length)
{
    memset(buffer, '?', max_len);
    ulong line_length = strlen(line);
    char* stopping = &(buffer[required_data_length]);
    char* p = buffer;
    while(true) {
        ulong x = sprintf(p, "%s", line);
        p = p + x;
        if((p - buffer) > required_data_length) {
            *p = (char)0;
            break;
        }
    }
    printf("done");
    size_t x = (p - buffer);
    return x;
}
void on_write_complete(void* ctx_arg, long bytes_written, int status)
{
    WriteCtxRef ctx = ctx_arg;
    runloop_timer_register(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = ctx_p_arg;
    RunloopStreamRef stream_ref = ctx->stream_ref;
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    SOCKW_CHECK_TAG(ctx->stream_ref)
    WTIMER_CHECK_TAG(ctx->timer_ref)
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait_timer_fired write_fd: %d", ctx->writefd);
    if(ctx->write_count > ctx->max_write_count) {
        asio_stream_close(ctx->asiostream_ref);
    } else {
        ctx->write_count++;
        char tmp[200];
        sprintf(tmp, "this is a line from writer %d count: %d\n", ctx->writer_index, ctx->write_count);
        size_t len = fill_buffer(tmp, ctx->outbuffer, ctx->outbuffer_max_length, 100000);
        asio_stream_write(ctx->asiostream_ref, ctx->outbuffer, strlen(ctx->outbuffer), &on_write_complete, ctx);
    }
}
void start_write(RunloopRef rl, void* ctx_arg)
{
    WriteCtxRef ctx = ctx_arg;
    sprintf(ctx->outbuffer, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
    asio_stream_write(ctx->asiostream_ref, ctx->outbuffer, strlen(ctx->outbuffer), &on_write_complete, ctx);
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);
        WriteCtx_set_stream_ref(ctx, runloop_ref, ctx->writefd);
        ctx->timer_ref = runloop_timer_new(runloop_ref);
        runloop_timer_register(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);

        RBL_CHECK_TAG(WriteCtx_ATG, ctx)
        RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
        WTIMER_CHECK_TAG(ctx->timer_ref);
        SOCKW_CHECK_TAG(ctx->stream_ref);
    }
    runloop_run(runloop_ref, 10000000);
    return NULL;
}
