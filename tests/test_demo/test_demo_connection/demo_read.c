
//#define RBL_LOG_ENABLED
//#define RBL_LOG_ALLOW_GLOBAL

#include "demo_read.h"
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
    this->input_list_ref = List_new();
    this->id = "READ";
    this->read_count = 0;
    this->max_read_count = max;
    this->readfd = fd;
    this->reader_index = my_index;
    this->inbuffer = malloc(200000);
    this->inbuffer_max_length = 200000;
    this->inbuffer_length = 0;
    this->demo_conn_ref = NULL;
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
void start_read(RunloopRef rl, void* ctx_arg);
void on_read_message(void* href, DemoMessageRef msg, int status)
{
    ReadCtxRef ctx = href;
    RBL_LOG_FMT("connection_ref: %p count: %d status: %d msg: %s", ctx->demo_conn_ref, ctx->read_count, status, IOBuffer_cstr(demo_message_serialize(msg)));
    ctx->read_count++;
    List_add_back(ctx->input_list_ref, msg);
    runloop_post(ctx->demo_conn_ref->runloop_ref, start_read, ctx);
    RBL_LOG_FMT("connection_ref: %p ctx->inut_list size: %d", ctx->demo_conn_ref, List_size(ctx->input_list_ref));
}
void start_read(RunloopRef rl, void* ctx_arg)
{
    ReadCtxRef ctx = ctx_arg;
    DemoConnectionRef cref = ctx->demo_conn_ref;
    democonnection_read(cref, on_read_message, ctx);
}
static void on_connection_complete(void* arg)
{

}
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    ReaderTable* rdr = (ReaderTable*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        ctx->demo_conn_ref = democonnection_new(runloop_ref, ctx->readfd, &on_connection_complete, ctx);
        runloop_post(runloop_ref, start_read, ctx);
    }
    runloop_run(runloop_ref, 1000000);
    RBL_LOG_FMT("after runloop_run()")
    return NULL;
}
