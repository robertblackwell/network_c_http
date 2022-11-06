#define _GNU_SOURCE
#define ENABLE_LOG
#include "io_write.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <c_http/logger.h>
#include <c_http/common/utils.h>
#include <c_http/async/types.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/w_timerfd.h>
#include <c_http/runloop/w_iofd.h>

/**
 * The writer does the following
 *
 *  wait on a timer to expire, then disarm the timer
 *  wait for the fd to become writeable and then write a show message in blocking mode
 *
 *  disarm writeable events
 *  rearm the timer
 *
 */
void WriteCtx_init(WriteCtx* this, int fd, WIoFdRef swatcher, WTimerFdRef twatcher, int max)
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
void Writer_dispose(Writer* this)
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
static void wrtr_wait(WTimerFdRef watch, void* arg, uint64_t event);
static void wrtr_cb(WIoFdRef sock_watch, void* arg, uint64_t event)
{
    ReactorRef reactor = sock_watch->runloop;
    XR_SOCKW_CHECK_TAG(sock_watch)
    WriteCtx* ctx = (WriteCtx*)(arg);
    LOG_FMT("test_io: Socket watcher wrtr_callback");

    char* wbuf = malloc(100);
    sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
    // synchronous write - assume the write buffers are big enough to take the entire message
    // we only got here if the fd is ready for a write
    int nwrite = write(sock_watch->fd, wbuf, strlen(wbuf));
    free(wbuf);
    ctx->write_count++;
    LOG_FMT("test_io: Socket watcher wrtr_callback fd: %d event : %lx nread: %d errno: %d write_count %d\n", sock_watch->fd,  event, nwrite, errno, ctx->write_count);
    if(ctx->write_count > ctx->max_write_count) {
        XrReactor_deregister(reactor, ctx->swatcher->fd);
        XrReactor_deregister(reactor, ctx->twatcher->fd);
        return;
    }
    // disarm writeable events on this fd
    WIoFd_disarm_write(sock_watch);
    WR_CTX_CHECK_TAG(ctx)
    XR_SOCKW_CHECK_TAG(ctx->swatcher)
    XR_WTIMER_CHECK_TAG(ctx->twatcher)
    // rearm the timer
    WTimerFd_rearm(ctx->twatcher);
}
static void wrtr_wait(WTimerFdRef watch, void* arg, uint64_t event)
{
    XR_WTIMER_CHECK_TAG(watch)
    LOG_FMT("test_io: Socket watcher wrtr_wait\n");
    WriteCtx* ctx = (WriteCtx*)(arg);
    WR_CTX_CHECK_TAG(ctx)
    XR_SOCKW_CHECK_TAG(ctx->swatcher)
    XR_WTIMER_CHECK_TAG(ctx->twatcher)
    LOG_FMT("test_io: Socket watcher wrtr_wait fd: %d event : %lx errno: %d\n", watch->fd,  event, errno);

    int write_here = 0;
    if(write_here) {
        char* wbuf = malloc(100);
        sprintf(wbuf, "this is a line from writer - %d\n", ctx->write_count);
        int nwrite = write(ctx->writefd, wbuf, strlen(wbuf));
        free(wbuf);
    } else {
        WTimerFd_disarm(watch);
        uint64_t interest = EPOLLERR | EPOLLOUT;
        WIoFd_arm_write(ctx->swatcher, &wrtr_cb, (void*) ctx);
    }
}
void* writer_thread_func(void* arg)
{
    int wait_first = 1;
    ReactorRef rtor_ref = XrReactor_new();
    Writer* wrtr = (Writer*)arg;
    for(int i = 0; i < wrtr->count; i++) {
        WriteCtx* ctx = &(wrtr->ctx_table[i]);

        wrtr->ctx_table[i].swatcher = WIoFd_new(rtor_ref, ctx->writefd);
        wrtr->ctx_table[i].twatcher = WTimerFd_new(rtor_ref, &wrtr_wait, (void*)ctx,  ctx->interval_ms, true);

        WR_CTX_CHECK_TAG(ctx)
        XR_WTIMER_CHECK_TAG(ctx->twatcher);
        XR_SOCKW_CHECK_TAG(ctx->swatcher);

        WIoFdRef sw = wrtr->ctx_table[i].swatcher;

        if(wait_first) {
            // register armed - wait 2 seconds
            // register disarmed - timer cb will arm it
            WIoFd_register(ctx->swatcher);
            WIoFd_arm_write(ctx->swatcher, &wrtr_cb, (void*)ctx);
        } else {

            uint64_t interest = EPOLLERR | EPOLLOUT;
            WIoFd_register(sw);
            WIoFd_arm_write(sw, &wrtr_cb, (void*) ctx);
        }
    }

    XrReactor_run(rtor_ref, 10000000);
    return NULL;
}
