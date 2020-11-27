#define _GNU_SOURCE
#include "io_read.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/epoll.h>
#include <math.h>
#include <c_http/list.h>
#include <c_http/operation.h>
#include <c_http/oprlist.h>
#include <c_http/unittest.h>
#include <c_http/utils.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/timer_watcher.h>
#include <c_http/xr/socket_watcher.h>

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
void Reader_add_fd(Reader* this, int fd)
{
    this->ctx_table[this->count].id = "READ";
    this->ctx_table[this->count].readfd = fd;
    this->count++;
}

static int read_count = 0;
void rd_callback(XrSocketWatcherRef watch, void* arg, uint64_t event)
{
    read_count++;
    ReadCtx* ctx = (ReadCtx*)arg;
    int in = event | EPOLLIN;
    char buf[1000];
    int nread = read(watch->fd, buf, 1000);
    char* s;
    if(nread > 0) {
        buf[nread] = (char)0;
        s = buf;
    } else {
        s = "badread";
    }
    XR_PRINTF("test_io: Socket watcher rd_callback read_count: %d fd: %d event %lx nread: %d buf: %s errno: %d\n", read_count, watch->fd,  event, nread, s, errno);
}
void* reader_thread_func(void* arg)
{
    XrReactorRef rtor_ref = XrReactor_new();
    Reader* rdr = (Reader*)arg;
    for(int i = 0; i < rdr->count; i++) {
        ReadCtx* ctx = &(rdr->ctx_table[i]);
        rdr->ctx_table[i].swatcher = Xrsw_new(rtor_ref, ctx->readfd);
        XrSocketWatcherRef sw = rdr->ctx_table[i].swatcher;
        uint64_t interest = EPOLLERR | EPOLLIN;
        Xrsw_register(sw, &rd_callback, (void*) ctx, 0);
        Xrsw_change_watch(sw, &rd_callback, (void*) ctx, interest);
    }
    XrReactor_run(rtor_ref, 1000000);
    return NULL;

}
