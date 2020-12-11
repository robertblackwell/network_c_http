#define _GNU_SOURCE
#define XR_TRACE_ENABLE
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
#include <c_http/xr/w_timer.h>
#include <c_http/xr/w_socket.h>

/**
 * The writer does the following
 *
 *  wait on a timer to expire, then disarm the timer
 *  wait for the fdd to become writeable and then write a show message in blocking mode
 *
 *  disarm writeable events
 *  rearm the timer
 *
 */
void WriteCtx_init(WriteCtx* this, int fd, WSocketRef swatcher, WTimerRef twatcher, int max)
{
    this->id = "WRITE";
    this->writefd = fd;
    this->twatcher = twatcher;
    this->swatcher = swatcher;
    this->write_count = 0;
    this->max_write_count = max;
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
void Writer_add_fd(Writer* this, int fd, int max, int interval_ms)
{
    WRTR_CHECK(this)
    WriteCtx* ctx = &(this->ctx_table[this->count]);
    this->ctx_table[this->count].write_count = 0;
    this->ctx_table[this->count].max_write_count = max;
    this->ctx_table[this->count].interval_ms = interval_ms;
    this->ctx_table[this->count].id = "WRITE";
    this->ctx_table[this->count].ctx_tag = WCTX_TAG;
    this->ctx_table[this->count].writefd = fd;
    this->count++;
}
static void wrtr_wait(WTimerRef watch, void* arg, uint64_t event);
static void wrtr_cb(WSocketRef sock_watch, void* arg, uint64_t event)
{
    XrReactorRef reactor = sock_watch->runloop;
    XRSW_TYPE_CHECK(sock_watch)
    WriteCtx* ctx = (WriteCtx*)(arg);
    XR_PRINTF("test_io: Socket watcher wrtr_callback");

    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
    // synchronous write - assume the write buffers are big enough to take the entire message
    // we only got here if the fd is ready for a write
    int nwrite = write(sock_watch->fd, wbuf, strlen(wbuf));
    free(wbuf);
    ctx->write_count++;
    XR_PRINTF("test_io: Socket watcher wrtr_callback fd: %d event : %lx nread: %d errno: %d write_count %d\n", sock_watch->fd,  event, nwrite, errno, ctx->write_count);
    if(ctx->write_count > ctx->max_write_count) {
        XrReactor_deregister(reactor, ctx->swatcher->fd);
        XrReactor_deregister(reactor, ctx->twatcher->fd);
        return;
    }
    // disarm writeable events on this fd
    WSocket_disarm_write(sock_watch);
    WR_CTX_CHECK_TAG(ctx)
    XRSW_TYPE_CHECK(ctx->swatcher)
    XRTW_TYPE_CHECK(ctx->twatcher)
    // rearm the timer
    WTimer_rearm(ctx->twatcher);
}
static void wrtr_wait(WTimerRef watch, void* arg, uint64_t event)
{
    XRTW_TYPE_CHECK(watch)
    XR_PRINTF("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = (WriteCtx*)(arg);
    WR_CTX_CHECK_TAG(ctx)
    XRSW_TYPE_CHECK(ctx->swatcher)
    XRTW_TYPE_CHECK(ctx->twatcher)
    XR_PRINTF("test_io: Socket watcher wrtr_wait fd: %d event : %lx errno: %d\n", watch->fd,  event, errno);

    int write_here = 0;
    if(write_here) {
        char* wbuf = malloc(100);
        sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
        int nwrite = write(ctx->writefd, wbuf, strlen(wbuf));
        free(wbuf);
    } else {
        WTimer_disarm(watch);
        uint64_t interest = EPOLLERR | EPOLLOUT;
        WSocket_arm_write(ctx->swatcher, &wrtr_cb, (void*) ctx);
    }
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    XrReactorRef rtor_ref = XrReactor_new();
    Writer* wrtr = (Writer*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);

        wrtr->ctx_table[i].swatcher = WSocket_new(rtor_ref, ctx->writefd);
        wrtr->ctx_table[i].twatcher = WTimer_new(rtor_ref, &wrtr_wait, (void*)ctx,  ctx->interval_ms, true);

        WR_CTX_CHECK_TAG(ctx)
        XRTW_TYPE_CHECK(ctx->twatcher);
        XRSW_TYPE_CHECK(ctx->swatcher);

        WSocketRef sw = wrtr->ctx_table[i].swatcher;

        if(wait_first) {
            // register armed - wait 2 seconds
            // register disarmed - timer cb will arm it
            WSocket_register(ctx->swatcher);
            WSocket_arm_write(ctx->swatcher, &wrtr_cb, (void*)ctx);
        } else {

            uint64_t interest = EPOLLERR | EPOLLOUT;
            WSocket_register(sw);
            WSocket_arm_write(sw, &wrtr_cb, (void*) ctx);
        }
    }

    XrReactor_run(rtor_ref, 10000000);
    return NULL;
}
