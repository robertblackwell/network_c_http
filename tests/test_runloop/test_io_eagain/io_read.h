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
#include <src/common/utils.h>
#include <src/runloop/runloop.h>

#define Reader_TAG "RDTag"
#define ReadCtx_TAG "RDRCtx"

typedef struct ReadCtx_s {
    RBL_DECLARE_TAG;
    int                 read_count;
    int                 max_read_count;
    char*               id;
    int                 reader_index;
    int                 readfd;
    void*               read_buffer;
    long                read_buffer_length;
    RunloopStreamRef    swatcher;
    AsioStreamRef       asio_stream_ref;
    RBL_DECLARE_END_TAG;
} ReadCtx;

typedef struct Reader_s {
    RBL_DECLARE_TAG;
    int     count;
    ReadCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} Reader;

void Reader_init(Reader* this);
Reader* Reader_new();
void Reader_free(Reader* this);
void Reader_add_fd(Reader* this, int fd, int max);

void rd_callback(RunloopStreamRef socket_watcher_ref, uint64_t event);
void* reader_thread_func(void* arg);
#endif