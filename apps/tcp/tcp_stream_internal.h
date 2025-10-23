#ifndef H_tcp_stream_internal_h
#define H_tcp_stream_internal_h
#ifdef __cplusplus
extern "C"
{
#endif
#include <common/iobuffer.h>
#include <runloop/runloop.h>
#include "tcp_stream.h"

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

typedef struct TcpReader_s {
    int                 read_state;
    // input_buffer is only not null when a read is pending or in progress
    // in such a state no new reads are allowed. assert() if it happens
    // When input_buffer != NULL it is required that read_cb != NULL
    IOBufferRef         input_buffer;
    TcpReadCallback*    read_cb;
    void*               read_cb_arg;

} TcpReader, *TcpReaderRef;

typedef struct TcpWriter_s {
    int                 write_state;
    // output_buffer is only not null when a write is pending or in progress
    // in such a state no new writes are allowed. assert() if it happens
    // When output_buffer != NULL it is required that write_cb != NULL
    TcpWriteCallback*   write_cb;
    void*               write_cb_arg;
    IOBufferRef         output_buffer;
} TcpWriter, *TcpWriterRef;

struct TcpListener_s {
    RBL_DECLARE_TAG;
    RunloopListenerRef  rl_listener_ref;
    TcpAcceptCallback*  accept_cb;
    void*               accept_cb_arg;
    int l_state;
    RBL_DECLARE_END_TAG;
};

struct TcpStream_s {
    RBL_DECLARE_TAG;
    RunloopStreamRef    rlstream_ref;
    int                 stream_fd;
    int                 my_table_index;
    TcpReader reader;
    TcpWriter writer;
    RBL_DECLARE_END_TAG;
};

#ifdef __cplusplus
}
#endif
#endif