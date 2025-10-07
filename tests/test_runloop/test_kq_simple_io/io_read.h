#ifndef c_http_tests_reactor_io_read_h
#define c_http_tests_reactor_io_read_h


#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#include <src/common/utils.h>
#include <kqueue_runloop/runloop.h>
#include <rbl/check_tag.h>
#define ReadCtx_TAG "rdctx"
#define ReaderTable_TAG "rdtbl"

#define RD_STATE_INITIAL 11
#define RD_STATE_EAGAIN 22
#define RD_STATE_STOPPED 33
#define RD_STATE_READY 44
#define RD_STATE_ERROR 55

typedef struct ReadCtx_s {
    RBL_DECLARE_TAG;
    int                 read_state;
    int                 return_code;
    int                 read_count;
    int                 max_read_count;
    char*               id;
    int                 reader_index;
    int                 readfd;
    RunloopEventRef     reader;
    RBL_DECLARE_END_TAG;
} ReadCtx, *ReadCtxRef;

void ReadCtx_init(ReadCtx* ctx, int my_index, int fd, int max);

typedef struct ReaderTable_s {
    RBL_DECLARE_TAG;
    int     count;
    ReadCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} ReaderTable, *ReaderTableRef;

void ReaderTable_init(ReaderTable* this);
ReaderTable* ReaderTable_new();
void ReaderTable_free(ReaderTableRef this);
void ReaderTable_add_fd(ReaderTable* this, int fd, int max);

void rd_callback(RunloopStreamRef socket_watcher_ref, uint64_t event);
void* reader_thread_func(void* arg);
#endif