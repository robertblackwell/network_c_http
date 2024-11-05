
//#define RBL_LOG_ENABLED
//#define RBL_LOG_ALLOW_GLOBAL
#include "io_write.h"
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
void WriteCtx_init(WriteCtx* this, int fd, RunloopStreamRef swatcher, RunloopTimerRef twatcher, int max)
{
    RBL_SET_TAG(WriteCtx_TAG, this);
    this->id = "WRITE";
    this->writefd = fd;
    this->twatcher = twatcher;
    this->swatcher = swatcher;
    this->write_count = 0;
    this->max_write_count = max;
    this->write_buffer = NULL;
    async_socket_set_nonblocking(fd);
    RBL_SET_END_TAG(WriteCtx_TAG, this);
}


void Writer_init(Writer* this)
{
    RBL_SET_TAG(Writer_TAG, this)
    this->count = 0;
    RBL_SET_END_TAG(Writer_TAG, this)
}
Writer* Writer_new()
{
    Writer* tmp = malloc(sizeof(Writer));
    Writer_init(tmp);
    return tmp;
}
void Writer_dispose(Writer* this)
{
    RBL_CHECK_TAG(Writer_TAG, this)
    RBL_CHECK_END_TAG(Writer_TAG, this)
    free(this);
}
void Writer_add_fd(Writer* this, int fd, int max, int interval_ms)
{
    RBL_CHECK_TAG(Writer_TAG, this)
    RBL_CHECK_END_TAG(Writer_TAG, this)
    async_socket_set_nonblocking(fd);

    WriteCtx* ctx = &(this->ctx_table[this->count]);
    RBL_CHECK_TAG(WriteCtx_TAG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_TAG, ctx)

    this->ctx_table[this->count].write_count = 0;
    this->ctx_table[this->count].max_write_count = max;
    this->ctx_table[this->count].interval_ms = interval_ms;
    this->ctx_table[this->count].id = "WRITE";
    this->ctx_table[this->count].writefd = fd;
    this->ctx_table[this->count].writer_index = this->count;
    this->count++;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* arg);
static long fill_buffer(char* line, char* buffer, int max_len, int required_data_length);
void write_done_callback(void* ctx_arg, long length, int error_number);

void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    Writer* wrtr = (Writer*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);
        RBL_CHECK_TAG(WriteCtx_TAG, ctx)
        RBL_CHECK_END_TAG(WriteCtx_TAG, ctx)
        wrtr->ctx_table[i].swatcher = runloop_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].twatcher = runloop_timer_new(runloop_ref);
        wrtr->ctx_table[i].asio_stream_ref = asio_stream_new(ctx->writefd, runloop_ref);
        runloop_timer_register(wrtr->ctx_table[i].twatcher, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);

        WTIMER_CHECK_TAG(ctx->twatcher);
        SOCKW_CHECK_TAG(ctx->swatcher);
    }

    runloop_run(runloop_ref, 10000000);
    return NULL;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = ctx_p_arg;
    RBL_CHECK_TAG(WriteCtx_TAG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_TAG, ctx)
    RunloopRef runloop_ref = ctx->swatcher->runloop;

    RunloopStreamRef stream_ref = ctx->swatcher;
    RunloopTimerRef timer_ref = ctx->twatcher;
    AsioStreamRef asio_stream_ref = ctx->asio_stream_ref;
    int fd = stream_ref->fd;
    SOCKW_CHECK_TAG(ctx->swatcher)
    WTIMER_CHECK_TAG(ctx->twatcher)
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait_timer_fired write_fd: %d", ctx->writefd);

    char* buffer = malloc(1000);
    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
    long buflen = fill_buffer(wbuf, buffer, 1000, 900);
    free(wbuf);
    ctx->write_buffer = buffer;
    asio_stream_write(asio_stream_ref, buffer, buflen, write_done_callback, ctx);
}
static long fill_buffer(char* line, char* buffer, int max_len, int required_data_length)
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
    return (long)strlen(buffer);
}
void write_done_callback(void* ctx_arg, long length, int error_number)
{
    WriteCtx* ctx = ctx_arg;
    RBL_CHECK_TAG(WriteCtx_TAG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_TAG, ctx)
    free(ctx->write_buffer);
    if(length <= 0) {
        assert(false);
    } else {
        ctx->write_count++;
        if(ctx->write_count < ctx->max_write_count) {
            // schedule timer to start next write
            runloop_timer_register(ctx->twatcher, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);
        } else {
            runloop_close(runloop_stream_get_reactor(ctx->swatcher));
        }
    }
}
