#ifndef c_http_tests_reactor_io_write_h
#define c_http_tests_reactor_io_write_h


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
#include <src/common/utils.h>
#include <src/runloop/runloop.h>
//#include <src/runloop/rl_internal.h>

#define Writer_TAG "12345678"
#define WriteCtx_TAG "WCTXTag"
#define WR_CTX_CHECK_TAG(w) assert(w->ctx_tag == WCTX_TAG);
#define WRTR_CHECK(w) assert(w->wrtr_tag == WRTR_TAG);

typedef struct WriteCtx_s {
    RBL_DECLARE_TAG;
    int                 write_count;
    int                 max_write_count;
    void*               write_buffer;
    char*               id;
    int                 writer_index;
    int                 writefd;
    int                 interval_ms;
    RunloopStreamRef    swatcher;
    RunloopTimerRef     twatcher;
    AsioStreamRef       asio_stream_ref;
    RBL_DECLARE_END_TAG;
} WriteCtx;

void WriteCtx_init(WriteCtx* this, int fd, RunloopStreamRef swatcher, RunloopTimerRef twatcher, int max);


typedef struct Writer_s {
    RBL_DECLARE_TAG;
    int     count;
    WriteCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} Writer;


void Writer_init(Writer* this);
Writer* Writer_new();
void Writer_free(Writer* this);
void Writer_add_fd(Writer* this, int fd, int max, int interval_ms);
void wrtr_callback(RunloopStreamRef watch, void* arg, uint64_t event);
void* writer_thread_func(void* arg);
#endif