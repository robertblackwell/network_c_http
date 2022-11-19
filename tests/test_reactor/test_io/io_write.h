#ifndef c_http_tests_reactor_io_write_h
#define c_http_tests_reactor_io_write_h

#define _GNU_SOURCE
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
#include <c_http/common/utils.h>
#include <c_http/simple_runloop/runloop.h>
#include <c_http/simple_runloop/rl_internal.h>

#define WRTR_TAG 123456789
#define WCTX_TAG 918273645
#define WR_CTX_CHECK_TAG(w) assert(w->ctx_tag == WCTX_TAG);
#define WRTR_CHECK(w) assert(w->wrtr_tag == WRTR_TAG);

typedef struct WriteCtx_s {
    int                 ctx_tag;
    int                 write_count;
    int                 max_write_count;
    char*               id;
    int                 writefd;
    int                 interval_ms;
    RtorStreamRef  swatcher;
    RtorTimerRef   twatcher;
} WriteCtx;
void WriteCtx_init(WriteCtx* this, int fd, RtorStreamRef swatcher, RtorTimerRef twatcher, int max);


typedef struct Writer_s {
    int     wrtr_tag;
    int     count;
    WriteCtx ctx_table[10];
} Writer;


void Writer_init(Writer* this);
Writer* Writer_new();
void Writer_dispose(Writer* this);
void Writer_add_fd(Writer* this, int fd, int max, int interval_ms);
void wrtr_callback(RtorStreamRef watch, void* arg, uint64_t event);
void* writer_thread_func(void* arg);
#endif