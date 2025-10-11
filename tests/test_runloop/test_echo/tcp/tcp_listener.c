#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <tcp/tcp_stream.h>
#include <kqueue_runloop/runloop.h>
#include <server/server_ctx.h>
#include <rbl/logger.h>

#define L_STATE_EAGAIN 22
#define L_STATE_READY 33
#define L_STATE_INITIAL 44
#define L_STATE_STOPPED 55
#define L_STATE_ERROR 66
static void on_timer(RunloopRef rl, void* arg);
static void on_accept_ready(RunloopRef rl, void* listener_ref_arg);
static void postable_try_accept(RunloopRef rl, void* arg);
static void try_accept(TcpListenerRef tcp_listener_ref);
static void on_accept_ready(RunloopRef rl, void* arg);
void invoke_accept_callback(TcpListenerRef t, int sock, int error);

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
// ServerCtx server_ctx;
// StreamTable stream_table;
// Server listener_ctx;
// static void server_main(RunloopRef runloop, void* arg);
// static void* accept_cb(void* arg, int sock, int error);

// int main() 
// {

//     int port = 9002;
//     int fd = local_create_bound_socket(port, "localhost");
//     socket_set_non_blocking(fd);
//     RunloopRef runloop = runloop_new();
//     listener_ctx_init(&(listener_ctx), fd, 1, runloop);
//     RBL_SET_TAG(ServerCtx_TAG, (&server_ctx))
//     RBL_SET_END_TAG(ServerCtx_TAG, (&server_ctx))
//     server_ctx.port = port;
//     server_ctx.listener_ctx_ref = &listener_ctx;
//     server_ctx.stream_table_ref = &stream_table;
//     ServerCtxRef server_ctx_ref = &server_ctx;
//     ServerRef listener_ref = &(listener_ctx);
//     runloop_post(runloop, server_main, server_ctx_ref);
//     runloop_run(runloop, NULL);
//     return 0;
// }

void tcp_accept(TcpListenerRef tcp_listener_ref, TcpAcceptCallback cb, void* arg)
{
    TcpListenerRef ctx = tcp_listener_ref;
    RBL_CHECK_TAG(TcpListener_TAG, ctx)
    RBL_CHECK_END_TAG(TcpListener_TAG, ctx)
    assert(ctx->accept_cb == NULL);
    assert(ctx->accept_cb_arg == NULL);
    ctx->accept_cb = cb;
    ctx->accept_cb_arg = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    RunloopRef rl = runloop_listener_get_runloop(ctx->rl_listener_ref);
    switch(ctx->l_state) {
        case L_STATE_INITIAL:
        case L_STATE_READY:
        case L_STATE_EAGAIN:
            if(ctx->accept_cb != NULL) {

            }                        
        case L_STATE_STOPPED:
        case L_STATE_ERROR:
            invoke_accept_callback(ctx, 0, -33);
            break;
        default:
            assert(0);
            break;
    }
    runloop_post(rl, postable_try_accept, ctx);
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
                printf("listen call failed with errno %d \n", errno);
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
#if 0
void* accept_cb(void* arg, int sock, int error)
{
    assert(arg != NULL);
    ServerCtxRef server = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, server)
    RBL_CHECK_END_TAG(ServerCtx_TAG, server)
    ServerRef listener_ctx = server->listener_ctx_ref;
    RBL_CHECK_TAG(Server_TAG, server)
    RBL_CHECK_END_TAG(Server_TAG, server)
    RunloopEventRef listener = listener_ctx->listener_event;
    RBL_CHECK_TAG(Listener_TAG, server)
    RBL_CHECK_END_TAG(Listener_TAG, server)
    RunloopRef rl = runloop_listener_get_runloop(listener);
    if(error == 0) {
        TcpStreamRef stream_ctx = StreamTable_add_fd(server->stream_table_ref, sock);
        stream_ctx->stream = runloop_stream_new(rl, sock);
        // have to start a stream handler as a new "green thread" which means post a function
        stream_handler_run(stream_ctx, stream_complete_cb, server);
        runloop_post(rl, try_accept, server);

    } else {
        // handle an error
    }
}
#endif
