#ifndef c_http_tests_test_reactor_listener_h
#define c_http_tests_test_reactor_listener_h


#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
// #include <src/sync/sync_client.h>
#include <kqueue_runloop/runloop.h>
#include <kqueue_runloop/runloop_internal.h>

#include <rbl/check_tag.h>
#define Ctx_TAG "LCTX"
#define RDCTX_TAG "RDCTX"
typedef int socket_handle_t;

void postable_reader_start(RunloopRef rl, void* arg);

typedef struct ReaderCtx_s {
    RBL_DECLARE_TAG;
    RunloopEventRef stream;
    int r_state;
    int rdfd;
    char line_buffer[1000];
    int line_buffer_length;
    int line_buffer_max;
    RBL_DECLARE_END_TAG;
} ReaderCtx, *ReaderCtxRef;

void reader_ctx_init(ReaderCtxRef rdctx, RunloopRef rl, int fd);

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
    RunloopEventRef         timer_ref;
    int                     listen_count;
    int                     accept_count;
    int                     max_accept_count;
    int                     id;
    
    // stuff for reader
    ReaderCtx               rdctx;
    RBL_DECLARE_END_TAG;
};
typedef struct  ListenerCtx_s TestServer, *ListenerCtxRef;


ListenerCtxRef listener_ctx_new(int listen_fd, int id, RunloopRef rl);
void listener_ctx_init(ListenerCtxRef sref, int listen_fd, int id, RunloopRef rl);

void listener_ctx_free(ListenerCtxRef sref);
void listener_ctx_run(ListenerCtxRef sref);

int local_create_bound_socket(int port, const char* host);


#endif