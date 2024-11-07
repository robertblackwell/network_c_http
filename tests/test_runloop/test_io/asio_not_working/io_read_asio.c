
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
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>

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
void ReadCtx_init(ReadCtx* ctx, int fd, int my_index, int max_count)
{
    ctx->id = "READ";
    ctx->read_count = 0;
    ctx->max_read_count = max_count;
    ctx->readfd = fd;
    ctx->reader_index = my_index;
    ctx->inbuf_max_length = 10000;
    ctx->inbuffer = malloc(ctx->inbuf_max_length);
    ctx->inbuf_length = 0;
    ctx->asio_stream_ref = NULL;
}

void ReaderTable_init(ReaderTableRef this)
{
    RBL_SET_TAG(ReaderTable_TAG, this)
    RBL_SET_END_TAG(ReaderTable_TAG, this)
    this->count = 0;
}
ReaderTableRef ReaderTable_new()
{
    ReaderTable* tmp = malloc(sizeof(ReaderTable));
    ReaderTable_init(tmp);
    return tmp;
}
void ReaderTable_dispose(ReaderTableRef this)
{
    free(this);
}
void ReaderTable_add_fd(ReaderTableRef this, int fd, int max)
{
    async_socket_set_nonblocking(fd);
    ReadCtxRef ctx = &(this->ctx_table[this->count]);
    ReadCtx_init(ctx, fd, this->count, max);
//    ctx->id = "READ";
//    ctx->read_count = 0;
//    ctx->max_read_count = max;
//    ctx->readfd = fd;
//    ctx->reader_index = this->count;
//    ctx->inbuf_max_length = 10000;
//    ctx->inbuffer = malloc(ctx->inbuf_max_length);
//    ctx->inbuf_length = 0;
//    ctx->asio_stream_ref = NULL;
    this->count++;

}
void start_read(RunloopRef rl, void* read_ctx_arg);
void read_callback(void* read_ctx_arg, long bytes_read, int status)
{
    ReadCtx* ctx = (ReadCtx*)read_ctx_arg;
    AsioStreamRef asio_ref = ctx->asio_stream_ref;
    RunloopRef runloop_ref = ctx->asio_stream_ref->runloop_ref;
    int fd = asio_ref->fd;
    if(bytes_read > 0) {
        ctx->inbuffer[bytes_read] = (char)0;
        char* s = &(ctx->inbuffer[0]);
        RBL_LOG_FMT("index: %d bytes_read:%ld fd: %d buf: %s errno: %d", ctx->reader_index, bytes_read, fd, s, status);
    } else {
        char* s = "badread";
        RBL_LOG_FMT("BAD READ errno: %d", status);
    }
    ctx->read_count++;
    memset(ctx->inbuffer, '?', bytes_read);
    ctx->inbuf_length = 0;
    runloop_post(runloop_ref, start_read, ctx);

}
void start_read(RunloopRef rl, void* read_ctx_arg)
{
    ReadCtxRef ctx = read_ctx_arg;
    asio_stream_read(ctx->asio_stream_ref, ctx->inbuffer, ctx->inbuf_max_length, &read_callback, ctx);
}
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    pid_t tid1 = gettid();
    ReaderTableRef rdr = (ReaderTableRef)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        ctx->asio_stream_ref = asio_stream_new(runloop_ref, ctx->readfd);
        runloop_post(runloop_ref, start_read, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
