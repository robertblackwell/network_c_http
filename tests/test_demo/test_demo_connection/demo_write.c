
//#define RBL_LOG_ENABLED
//#define RBL_LOG_ALLOW_GLOBAL
#include "demo_write.h"
#include "fill_iobuffer.h"

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
#include <http_in_c/demo_protocol/demo_connection.h>
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
    this->write_count = 0;
    this->writer_index = myindex;
    this->interval_ms = interval_ms;
    this->max_write_count = max;
    this->outbuffer = malloc(20000);
    this->outbuffer_max_length = 20000;
    this->outbuffer_length = 0;
    this->demo_conn_ref = NULL;
    async_socket_set_nonblocking(fd);
}
//void WriteCtx_set_stream_ref(WriteCtx* ctx, RunloopRef rl, int fd)
//{
//#if 0
//    ctx->stream_ref = runloop_stream_new(rl, fd);
//    ctx->asiostream_ref = NULL;
//#else
//    ctx->asiostream_ref = asio_stream_new(rl, fd);
//    ctx->stream_ref = ctx->asiostream_ref->runloop_stream_ref;
//#endif
//}


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
void WriterTable_dispose(WriterTable* this)
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

void on_write_complete(void* ctx_arg, int status)
{
    RBL_LOG_FMT("status: %d ctx_arg: %p", status, ctx_arg)
    WriteCtxRef ctx = ctx_arg;
    runloop_timer_register(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);
}
static void wrtr_wait_timer_fired(RunloopRef rl, void* ctx_p_arg)
{
    RBL_LOG_FMT("ctx_arg: %p", ctx_p_arg);
    WriteCtx* ctx = ctx_p_arg;
    DemoConnectionRef cref = ctx->demo_conn_ref;
    RBL_CHECK_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
    RBL_CHECK_TAG(DemoConnection_TAG, cref)
    WTIMER_CHECK_TAG(ctx->timer_ref)
    RBL_LOG_FMT("test_io: Socket watcher wrtr_wait_timer_fired write_fd: %d", ctx->writefd);
    if(ctx->write_count > ctx->max_write_count) {
        democonnection_close(cref);
    } else {
        ctx->write_count++;
        char tmp[200];
        sprintf(tmp, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
        DemoMessageRef msg = fill_demo_message(tmp, 1000, 30 );
        democonnection_write(cref, msg, on_write_complete, ctx);
    }
}
void start_write(RunloopRef rl, void* ctx_arg)
{
    WriteCtxRef ctx = ctx_arg;
    DemoConnectionRef cref = ctx->demo_conn_ref;
    char tmp[200];
    sprintf(tmp, "this is a line from writer %d count: %d", ctx->writer_index, ctx->write_count);
    DemoMessageRef msg = fill_demo_message(tmp, 1000, 30 );
    democonnection_write(cref, msg, on_write_complete, ctx);
}
static void on_connection_complete(void* arg_ctx)
{

}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    RunloopRef runloop_ref = runloop_new();
    WriterTable* wrtr = (WriterTable*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);
        ctx->demo_conn_ref = democonnection_new( runloop_ref, ctx->writefd,on_connection_complete, ctx);
        ctx->timer_ref = runloop_timer_new(runloop_ref);
        runloop_timer_register(ctx->timer_ref, &wrtr_wait_timer_fired, (void *) ctx, ctx->interval_ms, false);

        RBL_CHECK_TAG(WriteCtx_ATG, ctx)
        RBL_CHECK_END_TAG(WriteCtx_ATG, ctx)
        WTIMER_CHECK_TAG(ctx->timer_ref);
    }
    runloop_run(runloop_ref, 10000000);
    return NULL;
}
