#define _GNU_SOURCE
#define ENABLE_LOG

#include "io_read.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/epoll.h>
#include <c_http/logger.h>
#include <c_http/dsl/utils.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/w_socket.h>

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
void Reader_free(Reader* this)
{
    free(this);
}
void Reader_add_fd(Reader* this, int fd, int max)
{
    this->ctx_table[this->count].id = "READ";
    this->ctx_table[this->count].read_count = 0;
    this->ctx_table[this->count].max_read_count = max;
    this->ctx_table[this->count].readfd = fd;
    this->count++;

}

void rd_callback(WSocketRef socket_watcher_ref, void* arg, uint64_t event)
{
    XR_SOCKW_CHECK_TAG(socket_watcher_ref)
    ReadCtx* ctx = (ReadCtx*)arg;
    XrReactorRef reactor = socket_watcher_ref->runloop;
    int in = event | EPOLLIN;
    char buf[1000];
    int nread = read(socket_watcher_ref->fd, buf, 1000);
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = buf;
    } else {
        s = "badread";
    }
    LOG_FMT("test_io: Socket watcher rd_callback read_count: %d fd: %d event %lx nread: %d buf: %s errno: %d\n", ctx->read_count, socket_watcher_ref->fd,  event, nread, s, errno);
    ctx->read_count++;
    if(ctx->read_count > ctx->max_read_count) {
        XrReactor_deregister(reactor, socket_watcher_ref->fd);
    } else {
        return;
    }
}
void* reader_thread_func(void* arg)
{
    XrReactorRef rtor_ref = XrReactor_new();
    Reader* rdr = (Reader*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        rdr->ctx_table[i].swatcher = WSocket_new(rtor_ref, ctx->readfd);
        WSocketRef sw = rdr->ctx_table[i].swatcher;
        uint64_t interest = EPOLLERR | EPOLLIN;
        WSocket_register(sw);
        WSocket_arm_read(sw, &rd_callback, (void*) ctx);
//        WSocket_change_watch(sw, &rd_callback, (void*) ctx, interest);
    }

    XrReactor_run(rtor_ref, 1000000);
    return NULL;

}
