
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
void WriteCtx_init(WriteCtx* this, int fd, int my_index, int max_writes)
{
    RBL_SET_TAG(WriteCtx_TAG, this)
    RBL_SET_END_TAG(WriteCtx_TAG, this)
    this->id = "WRITE";
    this->writefd = fd;
    this->timer_ref = NULL;
    this->asio_stream_ref = NULL;
    this->writer_index = my_index;
    this->write_count = 0;
    this->outbuffer_max_length = 10000;
    this->outbuffer = malloc(this->outbuffer_max_length);
    this->max_write_count = max_writes;
    this->interval_ms = 1000;
    async_socket_set_nonblocking(fd);
}

void WriterTable_init(WriterTable* this)
{
    RBL_SET_TAG(WriteTable_TAG, this)
    RBL_SET_END_TAG(WriteTable_TAG, this)
    this->count = 0;
}
WriterTable* WriterTable_new()
{
    WriterTable* tmp = malloc(sizeof(WriterTable));
    WriterTable_init(tmp);
    return tmp;
}
void WriterTable_dispose(WriterTable* this)
{
    RBL_CHECK_TAG(WriteTable_TAG, this)
    RBL_CHECK_END_TAG(WriteTable_TAG, this)
    free(this);
}
void WriterTable_add_fd(WriterTable* this, int fd, int max, int interval_ms)
{
    RBL_CHECK_TAG(WriteTable_TAG, this)
    RBL_CHECK_END_TAG(WriteTable_TAG, this)

    async_socket_set_nonblocking(fd);

    WriteCtx* ctx = &(this->ctx_table[this->count]);
    WriteCtx_init(ctx, fd, this->count, max);
//    ctx->write_count = 0;
//    ctx->max_write_count = max;
//    ctx->interval_ms = interval_ms;
//    ctx->id = "WRITE";
//    ctx->writefd = fd;
//    ctx->writer_index = this->count;
//    ctx->outbuffer_length = 0;
    this->count++;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* arg);
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg);
static void wrtr_doit(RunloopRef rl, void* write_ctx_p_arg);
static void on_write_complete(void* write_ctx_arg, long bytes_written, int error);
static size_t fill_buffer(char* line, char* buffer, int max_len, int required_data_length);

void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    sleep(20);
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx *ctx = &(wrtr->ctx_table[i]);

        wrtr->ctx_table[i].asio_stream_ref = asio_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].timer_ref = runloop_timer_set(runloop_ref, &wrtr_wait_timer_fired, (void *) ctx,
                                                         ctx->interval_ms, true);
    }
    runloop_run(runloop_ref, 10000000);
    return NULL;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = ctx_p_arg;
    RunloopRef runloop_ref = ctx->asio_stream_ref->runloop_ref;
    AsioStreamRef asio_stream_ref = ctx->asio_stream_ref;
    RunloopTimerRef timer_ref = ctx->timer_ref;
    if(ctx->write_count >= ctx->max_write_count) {
        runloop_close(runloop_ref);
    } else {
        runloop_post(runloop_ref, wrtr_doit, ctx);
    }
}
static void wrtr_doit(RunloopRef rl, void* write_ctx_p_arg)
{
    WriteCtx* ctx = write_ctx_p_arg;
    AsioStreamRef asio_stream_ref = ctx->asio_stream_ref;
    RunloopRef runloop_ref = asio_stream_ref->runloop_ref;
    ctx->outbuffer_length = fill_buffer("Thisissomestufftoputinthebuffer", ctx->outbuffer, ctx->outbuffer_max_length, 100);
    asio_stream_write(asio_stream_ref, ctx->outbuffer, ctx->outbuffer_length, on_write_complete, ctx);
}
static void on_write_complete(void* write_ctx_arg, long bytes_written, int error)
{
    WriteCtx* ctx = write_ctx_arg;
    ctx->write_count++;
//    runloop_timer_reset(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);
    runloop_timer_deregister(ctx->timer_ref);
    runloop_timer_register(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);
}
static size_t fill_buffer(char* line, char* buffer, int max_len, int required_data_length)
{
    assert(buffer != NULL);
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
    size_t len = strlen(buffer);
    printf("done");
    return len;
}
