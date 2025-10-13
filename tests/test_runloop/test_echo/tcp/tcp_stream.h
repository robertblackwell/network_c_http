#ifndef H_tcp_stream_h
#define H_tcp_stream_h
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/runloop_internal.h>

#include <common/iobuffer.h>

// #define RD_STATE_INITIAL 11
// #define RD_STATE_EAGAIN 22
// #define RD_STATE_STOPPED 33
// #define RD_STATE_READY 44
// #define RD_STATE_ERROR 55
//
// #define WRT_STATE_EAGAIN 11
// #define WRT_STATE_READY 22
// #define WRT_STATE_ERROR 33
// #define WRT_STATE_STOPPED 44
// #define WRT_STATE_INITIAL 55

typedef void(TcpReadCallback)(void* arg, int error);
typedef void(TcpWriteCallback)(void* arg, int error);
typedef void(TcpAcceptCallback)(void* arg, int new_socket, int error);

#define TcpStream_TAG "TCPSTR"
#define TcpListener_TAG "TCPLSTN"

typedef int socket_handle_t;
typedef struct TcpReader_s TcpReader, *TcpReaderRef;
typedef struct TcpListener_s TcpListener, *TcpListenerRef;
typedef struct TcpStream_s TcpStream, *TcpStreamRef;

TcpStreamRef tcp_stream_new(RunloopRef rl, int fd);
void tcp_stream_init(TcpStreamRef tcp_stream_ref, RunloopRef rl, int fd);
void tcp_stream_deinit(TcpStreamRef tcp_stream_ref);
void tcp_stream_free(TcpStreamRef tcp_stream_ref);
void tcp_read(TcpStreamRef tcp, IOBufferRef iob, TcpReadCallback, void* arg);
RunloopRef tcp_stream_get_runloop(TcpStreamRef tcp_stream_ref);
/**
 * This function may modify the output buffer while framing it for transmission
 * but ownship remains with the caller
 */
void tcp_write(TcpStreamRef tcp, IOBufferRef iob, TcpWriteCallback, void* arg);
void tcp_close(TcpStreamRef tcp);

TcpListenerRef tcp_listener_new(RunloopRef rl, int fd);
void tcp_listener_init(TcpListenerRef listener_ref, RunloopRef rl, int fd);
void tcp_listener_deinit(TcpListenerRef listener_ref);
void tcp_listener_free(TcpListenerRef listener_ref);
void tcp_accept(TcpListenerRef tcp_listener_ref, TcpAcceptCallback cb, void* arg);
RunloopRef tcp_listener_get_runloop(TcpListenerRef tcp_listener_ref);

#endif