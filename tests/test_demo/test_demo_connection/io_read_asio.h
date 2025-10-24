#ifndef c_http_tests_runloop_io_read_asio_h
#define c_http_tests_runloop_io_read_asio_h


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
#define ReadCtx_TAG "rdctx"
#define ReaderTable_TAG "rdtbl"

typedef struct ReadCtx_s {
    RBL_DECLARE_TAG;
    int                 read_count;
    int                 max_read_count;
    char*               id;
    int                 reader_index;
    int                 readfd;
    char*               inbuffer;
    int                 inbuffer_max_length;
    int                 inbuffer_length;
    AsioStreamRef       asiostream_ref;
    RunloopStreamRef    stream_ref;

    RBL_DECLARE_END_TAG;
} ReadCtx, *ReadCtxRef;

void ReadCtx_init(ReadCtx* ctx, int my_index, int fd, int max);
void ReadCtx_set_stream_ref(ReadCtx* ctx, RunloopRef rl, int fd);

typedef struct ReaderTable_s {
    RBL_DECLARE_TAG;
    int     count;
    ReadCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} ReaderTable, *ReaderTableRef;

void ReaderTable_init(ReaderTable* this);
ReaderTable* ReaderTable_new();
void ReaderTable_dispose(ReaderTable* this);
void ReaderTable_add_fd(ReaderTable* this, int fd, int max);

void rd_callback(RunloopStreamRef socket_watcher_ref, uint64_t event);
void* reader_thread_func(void* arg);
#endif