
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
void ReadCtx_init(ReadCtx* this, int my_index, int fd, int max)
{
    RBL_SET_TAG(ReadCtx_TAG, this)
    RBL_SET_END_TAG(ReadCtx_TAG, this)
    this->id = "READ";
    this->read_count = 0;
    this->max_read_count = max;
    this->readfd = fd;
    this->reader_index = my_index;
    this->inbuffer = malloc(20000);
    this->inbuffer_max_length = 20000;
    this->inbuffer_length = 0;
    this->asiostream_ref = NULL;

}
void ReadCtx_set_stream_ref(ReadCtx* ctx, RunloopRef rl, int fd)
{
#if 1
    ctx->stream_ref = runloop_stream_new(rl, fd);
    ctx->asiostream_ref = NULL;
#else
    ctx->asiostream_ref = asio_stream_new(rl, fd);
    ctx->stream_ref = ctx->asiostream_ref->runloop_stream_ref;
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
void ReaderTable_dispose(ReaderTable* this)
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
void read_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    ReadCtx* ctx = (ReadCtx*)read_ctx_ref_arg;
    RunloopStreamRef stream = ctx->stream_ref;
    RunloopRef runloop_ref = runloop_stream_get_reactor(stream);
    char buf[100000];
    int x = sizeof(buf);
    memset(buf, 0, sizeof(buf));
    int fd = runloop_stream_get_fd(stream);
    int nread = read(fd, buf, 100000);
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = &(buf[0]);
        RBL_LOG_FMT("index: %d count:%d fd: %d buf: %s errno: %d", ctx->reader_index, ctx->read_count, fd, buf, errno);
    } else {
        s = "badread";
        RBL_LOG_FMT("BAD READ rd_callback read_count: %d fd: %d nread: %d buf: %s errno: %d", ctx->read_count,
                    fd, nread, s, errno);
    }
    ctx->read_count++;
    if(ctx->read_count > ctx->max_read_count) {
        runloop_deregister(runloop_ref, runloop_stream_get_fd(stream));
    } else {
        return;
    }
}
void start_read(RunloopRef rl, void* ctx_arg)
{
    ReadCtxRef ctx = ctx_arg;
    uint64_t interest = EPOLLERR | EPOLLIN;
    runloop_stream_register(ctx->stream_ref);
    runloop_stream_arm_read(ctx->stream_ref, &read_callback, (void *) ctx);
}
void timer_dummy(RunloopRef rl, void* arg)
{
    printf("timer_dummy");
    runloop_post(rl, start_read, arg);
//    assert(false);
}
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    ReaderTable* rdr = (ReaderTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        ReadCtx_set_stream_ref(ctx, runloop_ref, ctx->readfd);
        RunloopTimerRef t = runloop_timer_set(runloop_ref, timer_dummy, ctx, 100000, false);
        runloop_post(runloop_ref, start_read, ctx);
//        uint64_t interest = EPOLLERR | EPOLLIN;
//        runloop_stream_register(ctx->stream_ref);
//        runloop_stream_arm_read(ctx->stream_ref, &read_callback, (void *) ctx);
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
