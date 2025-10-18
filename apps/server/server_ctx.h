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
#include <common/utils.h>
#include <common/iobuffer.h>
#include <common/socket_functions.h>
#include <common/list.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/runloop_internal.h>
#include <rbl/check_tag.h>
#include <async_msg_stream/msg_stream.h>
#include <server_apps/server_app_selection_header.h>

#if 0
#ifdef APP_SELECT_ECHO
#include <server_apps/echo_app.h>
#elif APP_SELECT_DEMO
#include <server_apps/echo_app.h>
#else
#error "Server - no app was selected"
#endif
#endif

#define StreamTable_TAG "SRMTBL"
#define ServerCtx_TAG "SVRCTX"
#ifdef __cplusplus
extern "C" {
#endif

typedef int socket_handle_t;
typedef void(AppDoneCallback)(void* app, void* server, int error);
struct ServerCtx_s {
    RBL_DECLARE_TAG;
    int                     l_state;
    int                     port;
    const char*             host;
    // ServerAppInterfaceRef   app_interface;
    // application generic interface
    // void* (*app_new)(RunloopRef rl, int fd);
    // void (*app_run)(void* app, void(cb)(void* app, void* server, int error), void* arg);
    // void (*app_free)(void* app);

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

#ifdef __cplusplus
}
#endif
#endif