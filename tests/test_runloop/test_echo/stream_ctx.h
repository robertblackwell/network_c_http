#ifndef h_test_echo_io_stream_h
#define h_test_echo_io_stream_h


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
#include <common/iobuffer.h>
#include <common/socket_functions.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/runloop_internal.h>
#include <rbl/check_tag.h>
#define StreamCtx_TAG "STRMCTX"
#define StreamTable_TAG "SRMTBL"
#define Ctx_TAG "LCTX"

#define RD_STATE_INITIAL 11
#define RD_STATE_EAGAIN 22
#define RD_STATE_STOPPED 33
#define RD_STATE_READY 44
#define RD_STATE_ERROR 55

#define WRT_STATE_EAGAIN 11
#define WRT_STATE_READY 22
#define WRT_STATE_ERROR 33
#define WRT_STATE_STOPPED 44
#define WRT_STATE_INITIAL 55

typedef int socket_handle_t;

typedef void (*AsyncReadCallback) (void* arg, int nread, int async_read_errno);
typedef void (*AsyncWriteCallback) (void* arg, int nread, int async_read_errno);

typedef struct LineParser_s {
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
} LineParser, *LineParserRef;

void line_parser_init(LineParserRef lp);
char* line_parser_consume(LineParserRef lp, char* buffer, int buffer_length);

typedef struct ReaderCtx_s {
    int                 read_state;
    // input_buffer is only not null when a read is pending or in progress
    // in such a state no new reads are allowed. assert() if it happens
    // When input_buffer != NULL it is required that read_cb != NULL
    IOBufferRef         input_buffer;
    AsyncReadCallback   read_cb;
    void*               read_cb_arg;

    LineParser          line_parser;
} ReaderCtx;
typedef struct WriterCtx_s {
    int                 write_state;
    // output_buffer is only not null when a write is pending or in progress
    // in such a state no new writes are allowed. assert() if it happens
    // When output_buffer != NULL it is required that write_cb != NULL
    AsyncWriteCallback  write_cb;
    void*               write_cb_arg;   
    IOBufferRef         output_buffer;
} WriterCtx;

typedef struct StreamCtx {
    RBL_DECLARE_TAG;
    RunloopEventRef     stream;
    int                 stream_fd;
    int                 my_table_index;
    ReaderCtx reader;
    WriterCtx writer;
    RBL_DECLARE_END_TAG;
} StreamCtx, *StreamCtxRef;

void StreamCtx_init(StreamCtxRef ctx, int my_index, int fd);

typedef struct StreamTable_s {
    RBL_DECLARE_TAG;
    int     count;
    StreamCtx ctx_table[10];
    RBL_DECLARE_END_TAG;
} StreamTable, *StreamTableRef;

/**
 * Listener is an object that 
 * -    listens for an accepts connections, on a designated port.
 * -    holds accepted connection in a list.
 * -    terminates closing all connections after a specified period.
 * -    the timeinterval over which the object accepts connections is
 *      controlled by a w_timer
 * 
 */
struct ListenerCtx_s {
    RBL_DECLARE_TAG;
    int                     l_state;
    int                     port;
    const char*             host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    RunloopEventRef         rl_event;
    int                     listen_count;
    int                     accept_count;
    int                     max_accept_count;
    int                     id;
    StreamTable             stream_table;
    RBL_DECLARE_END_TAG;
};
typedef struct  ListenerCtx_s TestServer, *ListenerCtxRef;


ListenerCtxRef listener_ctx_new(int listen_fd, int id, RunloopRef rl);
void listener_ctx_init(ListenerCtxRef sref, int listen_fd, int id, RunloopRef rl);

void listener_ctx_free(ListenerCtxRef sref);
void listener_ctx_run(ListenerCtxRef sref);

int local_create_bound_socket(int port, const char* host);

void StreamTable_init(StreamTableRef stref);
StreamTable* StreamTable_new();
void StreamTable_free(StreamTableRef st);
StreamCtxRef StreamTable_add_fd(StreamTableRef st, int fd);

void postable_reader(RunloopRef rl, void* arg);

void* reader_thread_func(void* arg);

#endif