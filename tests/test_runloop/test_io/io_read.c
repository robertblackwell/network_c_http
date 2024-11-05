
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


void Reader_init(Reader* this)
{
    this->count = 0;
}
Reader* Reader_new()
{
    Reader* tmp = malloc(sizeof(Reader));
    Reader_init(tmp);
    return tmp;
}
void Reader_dispose(Reader* this)
{
    free(this);
}
void Reader_add_fd(Reader* this, int fd, int max)
{
    async_socket_set_nonblocking(fd);

    this->ctx_table[this->count].id = "READ";
    this->ctx_table[this->count].read_count = 0;
    this->ctx_table[this->count].max_read_count = max;
    this->ctx_table[this->count].readfd = fd;
    this->ctx_table[this->count].reader_index = this->count;
    this->count++;

}

void read_callback(RunloopRef rl, void* read_ctx_ref_arg)
{
    ReadCtx* ctx = (ReadCtx*)read_ctx_ref_arg;
    RunloopStreamRef stream = ctx->swatcher;
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
void* reader_thread_func(void* arg)
{
    RunloopRef runloop_ref = runloop_new();
    Reader* rdr = (Reader*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        rdr->ctx_table[i].swatcher = runloop_stream_new(runloop_ref, ctx->readfd);
        RunloopStreamRef sw = rdr->ctx_table[i].swatcher;
        uint64_t interest = EPOLLERR | EPOLLIN;
        runloop_stream_register(sw);
        runloop_stream_arm_read(sw, &read_callback, (void *) ctx);
    }
    runloop_run(runloop_ref, 1000000);
    return NULL;
}
