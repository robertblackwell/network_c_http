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

#include <sys/epoll.h>
#include <math.h>
#include <rbl/check_tag.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/runloop/runloop.h>

#define ReadCtx_TAG "rdctx"
#define ReaderTable_TAG "rdrtbl"

typedef struct ReadCtx_s {
    RBL_DECLARE_TAG;
    int                 read_count;
    int                 max_read_count;
    char*               id;
    int                 reader_index;
    int                 readfd;
    AsioStreamRef       asio_stream_ref;
    char*               inbuffer;
    int                 inbuf_max_length;
    int                 inbuf_length;
    RBL_DECLARE_END_TAG;
} ReadCtx, *ReadCtxRef;

void ReadCtx_init(ReadCtx* this, int fd, int my_index, int max_count);


typedef struct ReaderTable_s {
    RBL_DECLARE_TAG;
    int     count;
    ReadCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} ReaderTable, *ReaderTableRef;

void ReaderTable_init(ReaderTableRef this);
ReaderTableRef ReaderTable_new();
void ReaderTable_dispose(ReaderTableRef this);
void ReaderTable_add_fd(ReaderTableRef this, int fd, int max);

void rd_callback(RunloopStreamRef socket_watcher_ref, uint64_t event);
void* reader_thread_func(void* arg);
#endif