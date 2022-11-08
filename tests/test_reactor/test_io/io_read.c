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
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>

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
    this->ctx_table[this->count].id = "READ";
    this->ctx_table[this->count].read_count = 0;
    this->ctx_table[this->count].max_read_count = max;
    this->ctx_table[this->count].readfd = fd;
    this->count++;

}

void rd_callback(WIoFdRef socket_watcher_ref, void* arg, uint64_t event)
{
    WIoFd_verify(socket_watcher_ref);
    ReadCtx* ctx = (ReadCtx*)arg;
    ReactorRef reactor = WIoFd_get_reactor(socket_watcher_ref);
    int in = event | EPOLLIN;
    char buf[1000];
    int nread = read(WIoFd_get_fd(socket_watcher_ref), buf, 1000);
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = buf;
    } else {
        s = "badread";
    }
    LOG_FMT("test_io: Socket watcher rd_callback read_count: %d fd: %d event %lx nread: %d buf: %s errno: %d\n", ctx->read_count,
            WIoFd_get_fd(socket_watcher_ref),  event, nread, s, errno);
    ctx->read_count++;
    if(ctx->read_count > ctx->max_read_count) {
        XrReactor_deregister(reactor,  WIoFd_get_fd(socket_watcher_ref));
    } else {
        return;
    }
}
void* reader_thread_func(void* arg)
{
    ReactorRef rtor_ref = XrReactor_new();
    Reader* rdr = (Reader*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        rdr->ctx_table[i].swatcher = WIoFd_new(rtor_ref, ctx->readfd);
        WIoFdRef sw = rdr->ctx_table[i].swatcher;
        uint64_t interest = EPOLLERR | EPOLLIN;
        WIoFd_register(sw);
        WIoFd_arm_read(sw, &rd_callback, (void*) ctx);
//        WIoFd_change_watch(sw, &rd_callback, (void*) ctx, interest);
    }

    XrReactor_run(rtor_ref, 1000000);
    return NULL;

}
