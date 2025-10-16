#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <tcp/tcp_stream.h>
#include <tcp/tcp_stream_internal.h>
#include <kqueue_runloop/runloop.h>
#include <server/server_ctx.h>
#include <rbl/logger.h>

#define L_STATE_EAGAIN 22
#define L_STATE_READY 33
#define L_STATE_INITIAL 44
#define L_STATE_STOPPED 55
#define L_STATE_ERROR 66

static void on_accept_ready(RunloopRef rl, void* listener_ref_arg);
static void postable_try_accept(RunloopRef rl, void* arg);
static void try_accept(TcpListenerRef tcp_listener_ref);
void invoke_accept_callback(TcpListenerRef ctx, int new_sock, int error);

TcpListenerRef tcp_listener_new(RunloopRef rl, int fd)
{
    TcpListenerRef tlr = malloc(sizeof(TcpListenerRef));
    tcp_listener_init(tlr, rl, fd);
    return tlr;
}
void tcp_listener_init(TcpListenerRef tcp_listener_ref, RunloopRef rl, int fd)
{
    RBL_SET_TAG(TcpListener_TAG, tcp_listener_ref)
    RBL_SET_END_TAG(TcpListener_TAG, tcp_listener_ref)
    tcp_listener_ref->rl_listener_ref = runloop_listener_new(rl, fd);
    tcp_listener_ref->accept_cb = NULL;
    tcp_listener_ref->accept_cb_arg = NULL;
    tcp_listener_ref->l_state = L_STATE_INITIAL;
}
void tcp_listener_deinit(TcpListenerRef tcp_listener_ref)
{
    RBL_CHECK_TAG(TcpListener_TAG, tcp_listener_ref)
    RBL_CHECK_END_TAG(TcpListener_TAG, tcp_listener_ref)
    runloop_listener_deinit(tcp_listener_ref->rl_listener_ref);
}
void tcp_listener_free(TcpListenerRef tcp_listener_ref)
{
    RBL_CHECK_TAG(TcpListener_TAG, tcp_listener_ref)
    RBL_CHECK_END_TAG(TcpListener_TAG, tcp_listener_ref)
    runloop_listener_free(tcp_listener_ref->rl_listener_ref);
}
RunloopRef tcp_listener_get_runloop(TcpListenerRef tcp_listener_ref)
{
    return runloop_listener_get_runloop(tcp_listener_ref->rl_listener_ref);
}

void tcp_accept(TcpListenerRef tcp_listener_ref, TcpAcceptCallback cb, void* arg)
{
    TcpListenerRef ctx = tcp_listener_ref;
    RBL_CHECK_TAG(TcpListener_TAG, tcp_listener_ref)
    RBL_CHECK_END_TAG(TcpListener_TAG, tcp_listener_ref)
    assert(tcp_listener_ref->accept_cb == NULL);
    assert(tcp_listener_ref->accept_cb_arg == NULL);
    tcp_listener_ref->accept_cb = cb;
    tcp_listener_ref->accept_cb_arg = arg;
    RunloopRef rl = runloop_listener_get_runloop(tcp_listener_ref->rl_listener_ref);
    switch(tcp_listener_ref->l_state) {
        case L_STATE_INITIAL:
        case L_STATE_READY:
        case L_STATE_EAGAIN:
            if(tcp_listener_ref->accept_cb != NULL) {

            }
            break;                        
        case L_STATE_STOPPED:
        case L_STATE_ERROR:
            invoke_accept_callback(tcp_listener_ref, 0, -33);
            break;
        default:
            assert(0);
            break;
    }
    runloop_post(rl, postable_try_accept, tcp_listener_ref);
}
static void postable_try_accept(RunloopRef rl, void* arg)
{
    TcpListenerRef ctx = arg;
    RBL_CHECK_TAG(TcpListener_TAG, ctx)
    RBL_CHECK_END_TAG(TcpListener_TAG, ctx)
    RunloopListenerRef rllistener_ref = ctx->rl_listener_ref;
    runloop_listener_register(rllistener_ref, on_accept_ready, ctx);
    int fd = runloop_listener_get_fd(rllistener_ref);
    int result;
    switch(ctx->l_state) {
        case L_STATE_INITIAL:
            if((result = listen(fd, SOMAXCONN)) != 0) {
                printf("listen call failed with errno %d ", errno);
                assert(0);
            }
            ctx->l_state = L_STATE_READY;
            try_accept(ctx);
            break;
        case L_STATE_READY:
            try_accept(ctx);
            break;
        case L_STATE_EAGAIN:
            
        case L_STATE_STOPPED:
        case L_STATE_ERROR:
            break;
        default:
            assert(0);
            break;
    }
}
static void try_accept(TcpListenerRef tcp_listener_ref)
{
    RBL_CHECK_TAG(TcpListener_TAG, tcp_listener_ref)
    RBL_CHECK_END_TAG(TcpListener_TAG, tcp_listener_ref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    TcpListenerRef ctx = tcp_listener_ref;
    
    RunloopListenerRef rl_listener = ctx->rl_listener_ref;
    RunloopRef rl = runloop_listener_get_runloop(rl_listener);
    int fd = runloop_listener_get_fd(rl_listener);
    
    RBL_LOG_FMT("try_accept accept_fd: %d ", fd);
    
    int sock2 = accept(fd, (struct sockaddr *) &peername, &addr_length);
    int errno_saved = errno;
    if(sock2 > 0) {
        RBL_LOG_FMT("try_accept success sock2: %d", sock2);
        ctx->l_state = L_STATE_READY;
        invoke_accept_callback(ctx, sock2, 0);
    } else if (sock2 == 0) {
       // listen socket is closed ?? 
       // terminate the runloop ? 
       invoke_accept_callback(ctx, 0, -33);//TCP_ERR_EOF);
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_accept fd: %d EAGAIN", fd);
            ctx->l_state = L_STATE_EAGAIN;
            // make sure the runloop_listener is armed to catch the ready event
            // only on some platforms is ths necessary
            runloop_listener_arm(rl_listener, on_accept_ready, ctx);
        } else {
            ctx->l_state = L_STATE_ERROR;
            invoke_accept_callback(ctx, 0, errno_saved);
            // terminate the runloop
        }
    }
}

static void on_accept_ready(RunloopRef rl, void* arg)
{
    TcpListenerRef ctx = arg;
    RBL_CHECK_TAG(TcpListener_TAG, ctx)
    RBL_CHECK_END_TAG(TcpListener_TAG, ctx)
    RunloopListenerRef rl_listener = ctx->rl_listener_ref;
    RBL_LOG_FMT("on_read_ready fd: %d state: %d", runloop_listener_get_fd(rl_listener), ctx->l_state);
    switch(ctx->l_state) {
        case L_STATE_EAGAIN:
            ctx->l_state = L_STATE_READY;
        case L_STATE_READY:
            if(ctx->accept_cb != NULL) {
                try_accept(ctx);
            }
            break;
        case L_STATE_INITIAL:
        case L_STATE_STOPPED:
        case L_STATE_ERROR:
            RBL_LOG_FMT("on_event_listen should not be here fd: %d state: %d", runloop_listener_get_fd(rl_listener), ctx->l_state);
        default:
            assert(0);
            break;
    }
}
void invoke_accept_callback(TcpListenerRef ctx, int new_sock, int error)
{
    RBL_CHECK_TAG(TcpListener_TAG, ctx)
    RBL_CHECK_END_TAG(TcpListener_TAG, ctx)
    TcpAcceptCallback* cb = ctx->accept_cb;
    void* arg = ctx->accept_cb_arg;
    ctx->accept_cb = NULL;
    ctx->accept_cb_arg = NULL;
    cb(arg, new_sock, error);
}
