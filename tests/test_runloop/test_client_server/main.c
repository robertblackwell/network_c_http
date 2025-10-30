#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <server/server_ctx.h>

ServerCtx server_ctx;
static void server_main(RunloopRef runloop, void* arg);
static void* accept_cb(void* arg, int sock, int error);

int main() 
{

    int port = 9002;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);
    RunloopRef runloop = runloop_new();
    ServerCtxRef server_ctx_ref = &server_ctx;
    server_ctx_init(server_ctx_ref, runloop, fd);
    // RBL_SET_TAG(ServerCtx_TAG, (&server_ctx))
    // RBL_SET_END_TAG(ServerCtx_TAG, (&server_ctx))
    // server_ctx.port = port;
    // server_ctx.listener_ref = &listener_ctx;
    // ServerCtxRef server_ctx_ref = &server_ctx;
    // ServerRef listener_ref = &(listener_ctx);
    // runloop_post(runloop, server_main, server_ctx_ref);
    server_ctx_run(server_ctx_ref);
    runloop_run(runloop, 0L);
    return 0;
}
#if 0
void server_main(RunloopRef runloop, void* arg)
{
    ServerCtxRef server_ctx = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    ServerRef listener_ctx = server_ctx->listener_ctx_ref;
    listener_ctx_accept(listener_ctx, accept_cb, arg);
}
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
static void postable_try_listen(ServerRef server, RunloopRef rl)
{
    runloop_post(rl, try_accept, server);
}
void try_accept(RunloopRef rl, void* arg)
{
    ServerRef server_ctx = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    ServerRef listener_ctx = server_ctx->listener_ctx_ref;
    listener_ctx_accept(listener_ctx, accept_cb, arg);
}
void stream_complete_cb()
{

}
#endif