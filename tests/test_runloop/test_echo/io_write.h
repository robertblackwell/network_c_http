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
#include <math.h>
#include <src/common/utils.h>
#include <kqueue_runloop/runloop.h>
#include <rbl/check_tag.h>

#define WriterTable_TAG "WrtTbl"
#define WriteCtx_ATG "wrtCtx"

#define WRT_STATE_EAGAIN 11
#define WRT_STATE_READY 22
#define WRT_STATE_ERROR 33
#define WRT_STATE_STOPPED 44
#define WRT_STATE_INITIAL 55

typedef struct WriteCtx_s {
    RBL_DECLARE_TAG;
    int                 write_state;
    int                 write_count;
    int                 write_char_count;
    int                 max_write_count;
    char*               id;
    int                 writer_index;
    int                 writefd;
    int                 interval_ms;
    char*               outbuffer[10000];
    int                 outbuffer_max_length;
    int                 outbuffer_length;
    RunloopEventRef     writer_ref;
    RunloopEventRef     timer_ref;
    RBL_DECLARE_END_TAG;
} WriteCtx;
void WriteCtx_init(WriteCtx* this, int fd, RunloopStreamRef swatcher, RunloopEventRef twatcher, int max);


typedef struct WriterTable_s {
    RBL_DECLARE_TAG;
    int     count;
    WriteCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} WriterTable;


void WriterTable_init(WriterTable* this);
WriterTable* WriterTable_new();
void WriterTable_free(WriterTable* this);
void WriterTable_add_fd(WriterTable* this, int fd, int max, int interval_ms);
void wrtr_callback(RunloopStreamRef watch, void* arg, uint64_t event);
void* writer_thread_func(void* arg);
int WriterTable_char_count(WriterTable* wt);
#endif