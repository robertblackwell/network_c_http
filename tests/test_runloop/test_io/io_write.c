
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
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
// #include <src//runloop/rl_internal.h>
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
void WriteCtx_init(WriteCtx* this, int fd, RunloopStreamRef stream_ref, RunloopTimerRef timer_ref, int max)
{
    RBL_SET_TAG(WriteCtx_ATG, this)
    RBL_SET_END_TAG(WriteCtx_ATG, this)
    this->id = "WRITE";
    this->writefd = fd;
    this->timer_ref = timer_ref;
    this->stream_ref = stream_ref;
    this->write_count = 0;
    this->max_write_count = max;
    async_socket_set_nonblocking(fd);
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
    WriteCtx_init(ctx, fd, NULL, NULL, max);
    ctx->write_count = 0;
    ctx->max_write_count = max;
    ctx->interval_ms = interval_ms;
    ctx->id = "WRITE";
    ctx->writefd = fd;
    ctx->writer_index = this->count;
    ctx->outbuffer_length = 0;
    this->count++;
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* arg);

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
static void wrtr_cb(RunloopRef rl, void* write_ctx_p_arg)
{
    WriteCtx* ctx = write_ctx_p_arg;
    RunloopStreamRef stream = ctx->stream_ref;
    RunloopRef reactor = runloop_stream_get_runloop(stream);
    runloop_stream_verify(stream);
    int fd = runloop_stream_get_fd(stream);
#if 0
    char big_buffer[200000];
    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
    fill_buffer(wbuf, big_buffer, 200000, 190000);
    long bbuflen = strlen(big_buffer);
    // synchronous write - assume the write buffers are big enough to take the entire message
    // we only got here if the fd is ready for a write
    int nwrite = write(fd, big_buffer, strlen(big_buffer));
#else
    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
    int nwrite = write(fd, wbuf, strlen(wbuf));
#endif
    int eno = errno;
    int eag = EAGAIN;
    free(wbuf);
    RBL_LOG_FMT("index: %d fd: %d nread: %d errno: %d write_count %d\n", ctx->writer_index, fd, nwrite, errno, ctx->write_count);
    ctx->write_count++;
    if(ctx->write_count > ctx->max_write_count) {
        runloop_stream_deregister(stream);
        // runloop_deregister(reactor, runloop_stream_get_fd(ctx->stream_ref));
        runloop_timer_deregister(ctx->timer_ref);
        // runloop_deregister(reactor, runloop_timer_get_fd(ctx->timer_ref));
        return;
    }
    // disarm writeable events on this fd
    runloop_stream_disarm_write(stream);
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    runloop_stream_checktag(ctx->stream_ref);
    runloop_timer_checktag(ctx->timer_ref);
    // rearm the timer
    runloop_timer_rearm(ctx->timer_ref);
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = ctx_p_arg;
    RunloopRef runloop_ref = runloop_stream_get_runloop(ctx->stream_ref);
    RunloopStreamRef stream_ref = ctx->stream_ref;
    RunloopTimerRef timer_ref = ctx->timer_ref;
    int fd = runloop_stream_get_fd(stream_ref);
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    runloop_stream_checktag(ctx->stream_ref);
    runloop_timer_checktag(ctx->timer_ref);
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait_timer_fired write_fd: %d", ctx->writefd);

    int write_here = 0;
    if(write_here) {
        char* wbuf = malloc(100);
        sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
        int nwrite = write(ctx->writefd, wbuf, strlen(wbuf));
        free(wbuf);
    } else {
        runloop_timer_disarm(timer_ref);
        uint64_t interest = EPOLLERR | EPOLLOUT;
        runloop_stream_arm_write(ctx->stream_ref, &wrtr_cb, (void *) ctx);
    }
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);

        wrtr->ctx_table[i].stream_ref = runloop_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].timer_ref = runloop_timer_new(runloop_ref);
        runloop_timer_register(wrtr->ctx_table[i].timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);

        RBL_CHECK_TAG(WriteCtx_ATG, ctx)
        RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
        runloop_timer_checktag(ctx->timer_ref);
        runloop_stream_checktag(ctx->stream_ref);

        RunloopStreamRef sw = wrtr->ctx_table[i].stream_ref;

        if(wait_first) {
            // register armed - wait 2 seconds
            // register disarmed - timer cb will arm it
            runloop_stream_register(ctx->stream_ref);
            runloop_stream_arm_write(ctx->stream_ref, &wrtr_cb, (void *) ctx);
        } else {

            uint64_t interest = EPOLLERR | EPOLLOUT;
            runloop_stream_register(sw);
            runloop_stream_arm_write(sw, &wrtr_cb, (void *) ctx);
        }
    }

    runloop_run(runloop_ref, 10000000);
    return NULL;
}
