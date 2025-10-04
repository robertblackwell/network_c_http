
#define RBL_LOG_ENABLED
#define RBL_LOG_ALLOW_GLOBAL

#include "io_read_asio.h"
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
//#include <src/runloop/rl_internal.h>

void async_socket_set_nonblocking(int socket);

/**
 * the reader does the following
 *
 *  initialization is set a watcher to be called when the fd is readable
 *
 *      when readable read a message into a buffer
 *      print the message or if io error print "badread"
 *
 */
void ReadCtx_init(ReadCtx* this, int my_index, int fd, int max)
{
    RBL_SET_TAG(ReadCtx_TAG, this)
    RBL_SET_END_TAG(ReadCtx_TAG, this)
    this->id = "READ";
    this->read_count = 0;
    this->max_read_count = max;
    this->readfd = fd;
    this->reader_index = my_index;
    this->inbuffer = malloc(200000);
    this->inbuffer_max_length = 200000;
    this->inbuffer_length = 0;
    this->asiostream_ref = NULL;

}
void ReadCtx_set_stream_ref(ReadCtx* ctx, RunloopRef rl, int fd)
{
#if 0
    ctx->stream_ref = runloop_stream_new(rl, fd);
    ctx->asiostream_ref = NULL;
#else
    ctx->asiostream_ref = asio_stream_new(rl, fd);
    ctx->stream_ref = asio_stream_get_runloop(ctx->asiostream_ref);
#endif
}

void ReaderTable_init(ReaderTable* this)
{
    RBL_SET_TAG(ReaderTable_TAG, this)
    RBL_SET_END_TAG(ReaderTable_TAG, this)
    this->count = 0;
}
ReaderTable* ReaderTable_new()
{
    ReaderTable* tmp = malloc(sizeof(ReaderTable));
    ReaderTable_init(tmp);
    return tmp;
}
void ReaderTable_free(ReaderTable* this)
{
    RBL_CHECK_TAG(ReaderTable_TAG, this)
    RBL_CHECK_END_TAG(ReaderTable_TAG, this)
    free(this);
}
void ReaderTable_add_fd(ReaderTable* this, int fd, int max)
{
    RBL_CHECK_TAG(ReaderTable_TAG, this)
    RBL_CHECK_END_TAG(ReaderTable_TAG, this)
    async_socket_set_nonblocking(fd);
    ReadCtxRef ctx = &(this->ctx_table[this->count]);
    ReadCtx_init(ctx, this->count, fd, max);
    this->count++;

}
void start_read(RunloopRef rl, void* ctx_arg);
void on_read_data(void* ctx_arg, long bytes_read, int status)
{
    ReadCtxRef ctx = ctx_arg;
    ctx->inbuffer[bytes_read] = (char)0;
    if(bytes_read <= 100) {
        RBL_LOG_FMT("on_read_data bytes_read: %ld status: %d %s", bytes_read, status, ctx->inbuffer);
    } else {
        RBL_LOG_FMT("on_read_data bytes_read: %ld status: %d ", bytes_read, status);
    }
    if(bytes_read > 0) {
        ctx->read_count++;
        RunloopRef rl = asio_stream_get_runloop(ctx->asiostream_ref);
        runloop_post(rl, start_read, ctx);
    } else {
        asio_stream_close(ctx->asiostream_ref);
    }
}
void start_read(RunloopRef rl, void* ctx_arg)
{
    ReadCtxRef ctx = ctx_arg;
    asio_stream_read(ctx->asiostream_ref, ctx->inbuffer, ctx->inbuffer_max_length, on_read_data, ctx);
}
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    ReaderTable* rdr = (ReaderTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        ReadCtx_set_stream_ref(ctx, runloop_ref, ctx->readfd);
#if 1
        runloop_post(runloop_ref, start_read, ctx);
#else
        asio_stream_read(ctx->asiostream_ref, ctx->inbuffer, ctx->inbuffer_max_length, on_read_data, ctx);
#endif
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
