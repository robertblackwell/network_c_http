#ifndef h_test_echo_io_stream_h
#define h_test_echo_io_stream_h

#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <common/socket_functions.h>
#include <common/list.h>
#include <runloop/runloop.h>
// #include <runloop/runloop_internal.h>
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
    TcpListenerRef          tcp_listener_ref;
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