#include "io_write.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
void async_socket_set_nonblocking(int socket);
static void write_clean_terminate(WriteCtx* ctx);
static void write_error_terminate(WriteCtx* ctx);
static void try_write(WriteCtx* ctx);

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
void WriteCtx_init(WriteCtx* this, int fd, RunloopStreamRef stream_ref, RunloopEventRef timer_ref, int max)
{
    RBL_SET_TAG(WriteCtx_ATG, this)
    RBL_SET_END_TAG(WriteCtx_ATG, this)
    this->id = "WRITE";
    this->writefd = fd;
    this->timer_ref = timer_ref;
    this->writer_ref = stream_ref;
    this->write_count = 0;
    this->write_char_count = 0;
    this->max_write_count = max;
    this->write_state = WRT_STATE_STOPPED;
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
int WriterTable_char_count(WriterTable* rt)
{
    int count = 0;
    for(int i = 0; i < rt->count; i++) {
        count += rt->ctx_table[i].write_char_count;
    }
    return count;
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
// 
static void wrtr_cb(RunloopRef rl, void* write_ctx_p_arg)
{
    WriteCtx* ctx = write_ctx_p_arg;
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    RunloopStreamRef stream = ctx->writer_ref;
    RunloopRef reactor = runloop_stream_get_runloop(stream);
    runloop_stream_verify(stream);
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
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = ctx_p_arg;
    RunloopRef runloop_ref = runloop_stream_get_runloop(ctx->writer_ref);
    RunloopStreamRef stream_ref = ctx->writer_ref;
    RunloopEventRef timer_ref = ctx->timer_ref;
    int fd = runloop_stream_get_fd(stream_ref);
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    runloop_stream_checktag(ctx->writer_ref);
    runloop_timer_checktag(ctx->timer_ref);
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait_timer_fired write_fd: %d", ctx->writefd);
    switch(ctx->write_state) {
        case WRT_STATE_INITIAL:
            // first time
        case WRT_STATE_READY:
            try_write(ctx);
            break;
        case WRT_STATE_EAGAIN:
            // nothing to do
            break;
        case WRT_STATE_ERROR:
            // not sure yet
            break;
        case WRT_STATE_STOPPED:
            break;
        default:
            assert(false);
            break;
    }
}
static void try_write(WriteCtx* ctx)
{
    RBL_LOG_FMT("try_write fd: %d write_state: %d\n", ctx->writefd, ctx->write_state);
    RunloopEventRef timer_ref = ctx->timer_ref;
    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
    int nwrite = write(ctx->writefd, wbuf, strlen(wbuf));
    int errno_saved = errno;
    free(wbuf);
    if (nwrite > 0) {
        ctx->write_char_count += nwrite;
        if (ctx->write_count >= ctx->max_write_count) {
            RBL_LOG_FMT("try_write DONE fd: %d nwrite: %d write_count: %d\n", ctx->writefd, nwrite, ctx->write_count);
            close(ctx->writefd);
            write_clean_terminate(ctx);
        } else {
            RBL_LOG_FMT("try_write not done fd: %d nwrite: %d write_count: %d\n", ctx->writefd, nwrite, ctx->write_count);
            ctx->write_count++;
        }
    } else {
        if (errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_write EAGAIN fd: %d write_state: %d\n", ctx->writefd, ctx->write_state);
            ctx->write_state = WRT_STATE_EAGAIN;
        } else {
            RBL_LOG_FMT("try_write fd: %d errno %d desc: %s \n", ctx->writefd, errno_saved, strerror(errno_saved));
            write_error_terminate(ctx);
        }
    }    
}
static void write_clean_terminate(WriteCtx* ctx)
{   
    ctx->write_state = WRT_STATE_STOPPED;
    RunloopEventRef timer_ref = ctx->timer_ref;
    runloop_timer_disarm(timer_ref);
    runloop_timer_deregister(timer_ref);
    runloop_timer_free(timer_ref);
    
    runloop_stream_disarm_write(ctx->writer_ref);
    runloop_stream_deregister(ctx->writer_ref);
    runloop_stream_free(ctx->writer_ref);
}
static void write_error_terminate(WriteCtx* ctx)
{
    RunloopEventRef timer_ref = ctx->timer_ref;
    runloop_timer_disarm(timer_ref);
    runloop_timer_deregister(timer_ref);
    runloop_timer_free(timer_ref);
    runloop_stream_disarm_write(ctx->writer_ref);
    runloop_stream_deregister(ctx->writer_ref);
    runloop_stream_free(ctx->writer_ref);
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);
        ctx->write_state = WRT_STATE_INITIAL;
        wrtr->ctx_table[i].writer_ref = runloop_stream_new(runloop_ref, ctx->writefd);
        wrtr->ctx_table[i].timer_ref = runloop_timer_new(runloop_ref);
        // this timer will periodically write a message and when enough writes have happened close down the writer
        runloop_timer_register(wrtr->ctx_table[i].timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, true);

        RBL_CHECK_TAG(WriteCtx_ATG, ctx)
        RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
        runloop_timer_checktag(ctx->timer_ref);
        runloop_stream_checktag(ctx->writer_ref);

        runloop_stream_register(ctx->writer_ref);
        runloop_stream_arm_write(ctx->writer_ref, &wrtr_cb, (void *) ctx);
    }
    runloop_run(runloop_ref, 10000000);
    int total_chars = 0;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);
        total_chars += ctx->write_char_count;
        printf("Writer index: %d char count: %d \n", i, ctx->write_char_count);
    }
    printf("Writer total char count : %d\n", total_chars);
    return NULL;
}
