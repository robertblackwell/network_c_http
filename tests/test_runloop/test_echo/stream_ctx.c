#include "stream_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/logger.h>
#include <rbl/macros.h>
#include <src/common/utils.h>

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
void StreamCtx_init(StreamCtxRef ctx, int my_index, int fd)
{
    RBL_SET_TAG(StreamCtx_TAG, ctx)
    RBL_SET_END_TAG(StreamCtx_TAG, ctx)
    ctx->reader.read_state = RD_STATE_INITIAL;
    ctx->stream_fd = fd;
    ctx->my_table_index = my_index;
    ctx->stream = NULL;
    line_parser_init(&(ctx->reader.line_parser));
}

void StreamTable_init(StreamTableRef stref)
{
    RBL_SET_TAG(StreamTable_TAG, stref)
    RBL_SET_END_TAG(StreamTable_TAG, stref)
    stref->count = 0;
}
StreamTable *StreamTable_new()
{
    StreamTable* tmp = malloc(sizeof(StreamTable));
    StreamTable_init(tmp);
    return tmp;
}
void StreamTable_free(StreamTableRef st)
{
    RBL_CHECK_TAG(StreamTable_TAG, st)
    RBL_CHECK_END_TAG(StreamTable_TAG, st)
    free(st);
}
StreamCtxRef StreamTable_add_fd(StreamTableRef str, int fd)
{
    RBL_CHECK_TAG(StreamTable_TAG, str)
    RBL_CHECK_END_TAG(StreamTable_TAG, str)
    async_socket_set_nonblocking(fd);
    StreamCtxRef ctx = &(str->ctx_table[str->count]);
    StreamCtx_init(ctx, str->count, fd);
    str->count++;
    return ctx;
}
#if 0
void read_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    StreamCtx* ctx = (StreamCtx*)read_ctx_ref_arg;
    RBL_CHECK_TAG(StreamCtx_TAG, ctx)
    RBL_CHECK_END_TAG(StreamCtx_TAG, ctx)
    RunloopStreamRef stream = ctx->stream;
    RunloopRef runloop_ref = runloop_stream_get_runloop(stream);
    RBL_LOG_FMT("read_callback fd %d read_state: %d\n", ctx->stream_fd, ctx->reader.read_state);
    switch(ctx->reader.read_state){
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
        case RD_STATE_EAGAIN:
            ctx->reader.read_state = RD_STATE_READY;
            try_read(ctx);
            break;
        case RD_STATE_ERROR:
            break;
        case RD_STATE_STOPPED:
            break;
        default:
            assert(false);
    }
}
#endif
#if 0
static void try_read(StreamCtx* ctx)
{
    RunloopStreamRef stream = ctx->stream;
    RunloopRef runloop_ref = runloop_stream_get_runloop(stream);
    assert(ctx->stream_fd == runloop_stream_get_fd(stream));
    RBL_LOG_FMT("try_read: read_state %d fd: %d \n", ctx->reader.read_state, ctx->stream_fd);
    switch(ctx->reader.read_state) {
        case RD_STATE_EAGAIN:
        case RD_STATE_INITIAL:
        case RD_STATE_READY:
            break;
        case RD_STATE_ERROR:
        case RD_STATE_STOPPED:
            return;
    }
    char buf[100000];
    int x = sizeof(buf);
    memset(buf, 0, sizeof(buf));
    int fd = runloop_stream_get_fd(stream);
    int nread = read(fd, buf, 100000);
    int errno_saved = errno;
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = &(buf[0]);
        char* line = line_parser_consume(&(ctx->reader.line_parser), buf, nread);
        ctx->reader.read_state = RD_STATE_READY;
        RBL_LOG_FMT("try_read nread: %d read_state: %d buf:%s\n", nread, ctx->reader.read_state, buf);
        if(line) {
            RBL_LOG_FMT("try_read got a line: %s\n", line);
            line_parser_init(&(ctx->reader.line_parser));
        }
        runloop_post(runloop_ref, postable_reader, ctx);
    } else if (nread ==0) {
        // eof
        read_clean_termination(ctx);
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_read ERROR EAGAIN %s\n", "");
            ctx->reader.read_state = RD_STATE_EAGAIN;
            runloop_stream_arm_read(ctx->stream, read_callback, ctx);
        } else {
            RBL_LOG_FMT("try_read ERROR errno: %d description %s\n", errno_saved, strerror(errno_saved));
            read_error_termination(ctx, errno_saved);
        }
    }
}
void postable_reader(RunloopRef rl, void* arg)
{
    StreamCtx* ctx = arg;
    try_read(ctx);
}
static void read_clean_termination(StreamCtx* ctx)
{
    RBL_LOG_FMT("read_clean_termination fd %d read_state: %d\n", ctx->stream_fd, ctx->reader.read_state);
    ctx->reader.read_state = RD_STATE_STOPPED;
    runloop_stream_deregister(ctx->stream);
    runloop_stream_disarm_read(ctx->stream);
    runloop_stream_free(ctx->stream);
}
static void read_error_termination(StreamCtx* ctx, int err)
{
    RBL_LOG_FMT("read_error_termination fd %d read_state: %d \n", ctx->stream_fd, ctx->reader.read_state);
    ctx->reader.read_state = RD_STATE_ERROR;
    runloop_stream_deregister(ctx->stream);
    runloop_stream_disarm_read(ctx->stream);
    runloop_stream_free(ctx->stream);
}
    #endif
#if 0
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    StreamTable* rdr = (StreamTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        StreamCtx* ctx = &(rdr->ctx_table[i]);
        RBL_LOG_FMT("read_thread_func loop i: %d fd %d read_state: %d\n", i, ctx->readfd, ctx->read_state);

        rdr->ctx_table[i].reader = runloop_stream_new(runloop_ref, ctx->readfd);
        RunloopStreamRef sw = rdr->ctx_table[i].reader;
        // runloop_stream_register(sw);
        // runloop_stream_arm_read(sw, &read_callback, (void *) ctx);
        runloop_post(runloop_ref, postable_reader, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    int total_chars = 0;
    for(int i = 0; i < rdr->count; i++) {
        StreamCtx* ctx = &(rdr->ctx_table[i]);
        total_chars += ctx->read_char_count;
        printf("Reader index: %d char count: %d \n", i, ctx->read_char_count);
    }
    printf("Reader total char count : %d\n", total_chars);

    return NULL;
}
#endif
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, modFlags2);
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set non blocking error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    
    RBL_ASSERT((result == 0), "set socket non blocking");
}
