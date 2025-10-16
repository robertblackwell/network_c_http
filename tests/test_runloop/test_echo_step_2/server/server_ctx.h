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
#include <common/list.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/runloop_internal.h>
#include <rbl/check_tag.h>
#include <async_msg_stream/msg_stream.h>

#define StreamTable_TAG "SRMTBL"
#define ServerCtx_TAG "SVRCTX"

typedef int socket_handle_t;
typedef void(AppDoneCallback)(void* app, void* server, int error);
struct ServerCtx_s {
    RBL_DECLARE_TAG;
    int                     l_state;
    int                     port;
    const char*             host;
    socket_handle_t         listening_socket_fd;
    RunloopRef              runloop_ref;
    // actual struct not pointer
    TcpListenerRef          tcp_listener_ref;
    // RunloopListenerRef      rl_listener_ref;
    // this is only here to allocate space always use the _ref
    // RunloopListener         memory_rl_listener;
    // This list keeps track of open connections/sockets 
    ListRef                 connection_list;
    RBL_DECLARE_END_TAG;
};
typedef struct  ServerCtx_s ServerCtx, *ServerCtxRef;

ServerCtxRef server_ctx_new(RunloopRef rl, int listener_fd);
void server_ctx_init(ServerCtxRef server_ref, RunloopRef rl, int listener_fd);

void server_ctx_free(ServerCtxRef sref);
void server_ctx_run(ServerCtxRef sref);
void server_ctx_accept(ServerCtxRef lctx, void(accept_callback)(void* arg, int new_socket, int error), void* arg);

int local_create_bound_socket(int port, const char* host);

void postable_reader(RunloopRef rl, void* arg);

void* reader_thread_func(void* arg);

#endif