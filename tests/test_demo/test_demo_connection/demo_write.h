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
#include <rbl/check_tag.h>
#include <src/demo_protocol/demo_connection.h>

#define WriterTable_TAG "WrtTbl"
#define WriteCtx_ATG "wrtCtx"

typedef struct WriteCtx_s {
    RBL_DECLARE_TAG;
    int                 write_count;
    int                 max_write_count;
    char*               id;
    int                 writer_index;
    int                 writefd;
    int                 interval_ms;
    char*               outbuffer;
    int                 outbuffer_max_length;
    int                 outbuffer_length;
    DemoConnectionRef   demo_conn_ref;
    RunloopTimerRef     timer_ref;
    RBL_DECLARE_END_TAG;
} WriteCtx, *WriteCtxRef;
void WriteCtx_init(WriteCtx* this, int fd, int myindex, int max, int write_interval_ms);
void WriteCtx_set_stream_ref(WriteCtx* ctx, RunloopRef rl, int fd);

typedef struct WriterTable_s {
    RBL_DECLARE_TAG;
    int     count;
    WriteCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} WriterTable, *WriterTableRef;


void WriterTable_init(WriterTable* this);
WriterTable* WriterTable_new();
void WriterTable_dispose(WriterTable* this);
void WriterTable_add_fd(WriterTable* this, int fd, int max, int interval_ms);
void wrtr_callback(RunloopStreamRef watch, void* arg, uint64_t event);
void* writer_thread_func(void* arg);
#endif