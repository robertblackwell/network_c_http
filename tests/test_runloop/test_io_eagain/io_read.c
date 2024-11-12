
#define RBL_LOG_ENABLED
#define RBL_LOG_ALLOW_GLOBAL

#include "io_read.h"
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
 * the reader does the following
 *
 *  initialization is set a watcher to be called when the fd is readable
 *
 *      when readable read a message into a buffer
 *      print the message or if io error print "badread"
 *
 */


void Reader_init(Reader* this)
{
    RBL_SET_TAG(Reader_TAG, this);
    RBL_SET_END_TAG(Reader_TAG, this)
    this->count = 0;
}
Reader* Reader_new()
{
    Reader* tmp = malloc(sizeof(Reader));
    Reader_init(tmp);
    return tmp;
}
void Reader_free(Reader* this)
{
    free(this);
}
void read_ctx_init(ReadCtx* ctx, int fd, int max_reads, int index)
{
    RBL_SET_TAG(ReadCtx_TAG, ctx)
    RBL_SET_END_TAG(ReadCtx_TAG, ctx)
    ctx->id = "READ";
    ctx->read_count = 0;
    ctx->max_read_count = max_reads;
    ctx->readfd = fd;
    ctx->reader_index = index;
}
void Reader_add_fd(Reader* this, int fd, int max)
{
    async_socket_set_nonblocking(fd);
    ReadCtx* ctx = &(this->ctx_table[this->count]);
    read_ctx_init(ctx, fd, max, this->count);
    this->count++;
}
void read_start(RunloopRef rl, void* read_ctx_arg);
void read_done(void* read_ctx_arg, long length, int error_number);
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    Reader* rdr = (Reader*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        ctx->swatcher = runloop_stream_new(runloop_ref, ctx->readfd);
        ctx->asio_stream_ref = asio_stream_new(ctx->readfd, runloop_ref);
        runloop_post(runloop_ref, read_start, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
void read_done(void* read_ctx_arg, long length, int error_number)
{
    ReadCtx* ctx = read_ctx_arg;
    RBL_CHECK_TAG(ReadCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ReadCtx_TAG, ctx)
    RunloopRef rl = ctx->asio_stream_ref->runloop_ref;
    if(length > 0) {
        RBL_LOG_FMT("")
        runloop_post(rl, read_start, ctx);
    } else {
        RBL_LOG_MSG("read length <= 0")
        runloop_close(rl);
    }
}
void read_start(RunloopRef rl, void* read_ctx_arg)
{
    ReadCtx* ctx = read_ctx_arg;
    RBL_CHECK_TAG(ReadCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ReadCtx_TAG, ctx)
    char* buffer = malloc(2000);
    asio_stream_read(ctx->asio_stream_ref, buffer, 2000, read_done, ctx);
}
