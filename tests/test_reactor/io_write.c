#define _GNU_SOURCE
#include "io_write.h"
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
#include <c_http/utils.h>
#include <c_http/xr/types.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/timer_watcher.h>
#include <c_http/xr/socket_watcher.h>


void WriteCtx_init(WriteCtx* this, int fd, XrSocketWatcherRef swatcher, XrTimerWatcherRef twatcher)
{
    this->id = "WRITE";
    this->writefd = fd;
    this->twatcher = twatcher;
    this->swatcher = swatcher;
}


void Writer_init(Writer* this)
{
    this->wrtr_tag = WRTR_TAG;
    this->count = 0;
}
Writer* Writer_new()
{
    Writer* tmp = malloc(sizeof(Writer));
    Writer_init(tmp);
    return tmp;
}
void Writer_free(Writer* this)
{
    WRTR_CHECK(this)
    free(this);
}
void Writer_add_fd(Writer* this, int fd)
{
    WRTR_CHECK(this)
    WriteCtx* ctx = &(this->ctx_table[this->count]);

    this->ctx_table[this->count].id = "WRITE";
    this->ctx_table[this->count].ctx_tag = WCTX_TAG;
    this->ctx_table[this->count].writefd = fd;
    this->count++;
}
static void wrtr_wait(XrTimerWatcherRef watch, void* arg, uint64_t event);
static int write_count = 0;
static void wrtr_cb(XrSocketWatcherRef watch, void* arg, uint64_t event)
{
    XRSW_TYPE_CHECK(watch)
    XR_PRINTF("test_io: Socket watcher wrtr_callback");

    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer - %d\n", write_count);
    int nwrite = write(watch->fd, wbuf, strlen(wbuf));
    free(wbuf);

    XR_PRINTF("test_io: Socket watcher wrtr_callback fd: %d event : %lx nread: %d errno: %d\n", watch->fd,  event, nwrite, errno);
    Xrsw_change_watch(watch, NULL, NULL, 0);
    WriteCtx* ctx = (WriteCtx*)(arg);
    WR_CTX_CHECK_TAG(ctx)
    XRSW_TYPE_CHECK(ctx->swatcher)
    XRTW_TYPE_CHECK(ctx->twatcher)
    Xrtw_rearm(ctx->twatcher, wrtr_wait, (void*)ctx,  2000, true);
}
static void wrtr_wait(XrTimerWatcherRef watch, void* arg, uint64_t event)
{
    XRTW_TYPE_CHECK(watch)
    write_count++;
    XR_PRINTF("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = (WriteCtx*)(arg);
    WR_CTX_CHECK_TAG(ctx)
    XRSW_TYPE_CHECK(ctx->swatcher)
    XRTW_TYPE_CHECK(ctx->twatcher)
    XR_PRINTF("test_io: Socket watcher wrtr_wait fd: %d event : %lx errno: %d\n", watch->fd,  event, errno);

    int write_here = 0;
    if(write_here) {
        char* wbuf = malloc(100);
        sprintf(wbuf, "this is a line from writer - %d\n", write_count);
        int nwrite = write(ctx->writefd, wbuf, strlen(wbuf));
        free(wbuf);
    } else {
        Xrtw_disarm(watch);
        uint64_t interest = EPOLLERR | EPOLLOUT;
        Xrsw_change_watch(ctx->swatcher, &wrtr_cb, (void*) ctx, interest);
    }
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    XrReactorRef rtor_ref = XrReactor_new();
    Writer* wrtr = (Writer*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);

        wrtr->ctx_table[i].swatcher = Xrsw_new(rtor_ref, ctx->writefd);
        wrtr->ctx_table[i].twatcher = Xrtw_new(rtor_ref);

        WR_CTX_CHECK_TAG(ctx)
        XRTW_TYPE_CHECK(ctx->twatcher);
        XRSW_TYPE_CHECK(ctx->swatcher);

        XrSocketWatcherRef sw = wrtr->ctx_table[i].swatcher;

        if(wait_first) {
            // register armed - wait 2 seconds
            Xrtw_set(ctx->twatcher, &wrtr_wait, (void*)ctx,  2000, true);
            // register disarmed - timer cb will arm it
            Xrsw_register(ctx->swatcher, &wrtr_cb, (void*)ctx, 0);
        } else {
            uint64_t interest = EPOLLERR | EPOLLOUT;
            Xrsw_register(sw, &wrtr_cb, (void*) ctx, interest);
        }
    }

    XrReactor_run(rtor_ref, 10000000);
    return NULL;
}
