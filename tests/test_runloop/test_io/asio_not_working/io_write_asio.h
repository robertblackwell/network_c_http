#ifndef c_http_tests_runloop_io_write_asio_h
#define c_http_tests_runloop_io_write_asio_h


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
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/runloop/rl_internal.h>

#define WriteTable_TAG "WrtTbl"
#define WriteCtx_TAG "WrtCtx"

typedef struct WriteCtx_s {
    RBL_DECLARE_TAG;
    int                 write_count;
    int                 max_write_count;
    char*               id;
    int                 writer_index;
    int                 writefd;
    int                 interval_ms;
    char*               outbuffer;
    size_t              outbuffer_max_length;
    size_t              outbuffer_length;
    AsioStreamRef       asio_stream_ref;
    RunloopTimerRef     timer_ref;
    RBL_DECLARE_END_TAG;
} WriteCtx, *WriteCtxRef;

void WriteCtx_init(WriteCtx* this, int fd, int myindex, int max_writes);


typedef struct WriterTable_s {
    RBL_DECLARE_TAG;
    int     count;
    WriteCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} WriterTable, *WriterTableRef;


void WriterTable_init(WriterTableRef this);
WriterTable* WriterTable_new();
void WriterTable_dispose(WriterTable* this);
void WriterTable_add_fd(WriterTable* this, int fd, int max, int interval_ms);
void wrtr_callback(RunloopStreamRef watch, void* arg, uint64_t event);
void* writer_thread_func(void* arg);
#endif