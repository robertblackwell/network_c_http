#include <src/runloop/runloop.h>
#include "server_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include "server_ctx.h"

#define L_STATE_EAGAIN 22
#define L_STATE_READY 33
#define L_STATE_INITIAL 44
#define L_STATE_STOPPED 55
#define L_STATE_ERROR 66

static void handle_new_socket(void* server, int sock, int error);
static void postable_start(RunloopRef rl, void* arg);
static void app_instance_done_cb(void* app, void* server, int error);


ServerCtxRef server_ctx_new(RunloopRef rl, int listener_fd, ServerAppInterfaceRef app_interface)
{
    ServerCtxRef sref = malloc(sizeof(ServerCtx));
    server_ctx_init(sref, rl, listener_fd, app_interface);
    RBL_SET_TAG(ServerCtx_TAG, sref)
    RBL_SET_END_TAG(ServerCtx_TAG, sref)
    return sref;
}

void server_ctx_init(ServerCtxRef server_ctx, RunloopRef rl, int fd, ServerAppInterfaceRef app_interface)
{
    RBL_SET_TAG(ServerCtx_TAG, server_ctx)
    RBL_SET_END_TAG(ServerCtx_TAG, server_ctx)
    server_ctx->runloop_ref = rl;
    server_ctx->app_interface = app_interface;
    server_ctx->tcp_listener_ref = tcp_listener_new(server_ctx->runloop_ref, fd);
    // runloop_listener_init(server_ctx->rl_listener_ref, server_ctx->runloop_ref, fd);
    server_ctx->l_state = L_STATE_INITIAL;
    server_ctx->connection_list = List_new();
}

void server_ctx_deinit(ServerCtxRef server_ctx)
{
    server_ctx->tcp_listener_ref = NULL;
    runloop_free(server_ctx->runloop_ref);
    List_safe_free(server_ctx->connection_list, free);
}

void server_ctx_free(ServerCtxRef sref)
{
    ASSERT_NOT_NULL(sref);
    RBL_CHECK_TAG(ServerCtx_TAG, sref)
    RBL_CHECK_END_TAG(ServerCtx_TAG, sref)
    free(sref);
}
void server_ctx_run(ServerCtxRef ctx)
{
    ASSERT_NOT_NULL(ctx)
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    runloop_post(ctx->runloop_ref, postable_start, ctx);    
    // int nn = event_table_number_in_use(ctx->runloop_ref->event_table);
    // runloop_run(ctx->runloop_ref, -1);
    // printf("Listener runloop ended \n");
    // runloop_free(ctx->runloop_ref);
}

static void postable_start(RunloopRef rl, void* arg)
{
    ServerCtxRef server_ctx = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, server_ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, server_ctx)
    TcpListenerRef tcplr = server_ctx->tcp_listener_ref;
    tcp_accept(tcplr, handle_new_socket, server_ctx);    
}
static void handle_new_socket(void* server, int new_sock, int error)
{
    assert(new_sock != 0);
    ServerCtxRef ctx = server;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    // int nbsock = socket_set_blocking(sock);
    RBL_LOG_FMT("handle_new_socket ctx: %p sock: %d ", ctx, new_sock)
    RunloopRef rl = tcp_listener_get_runloop(ctx->tcp_listener_ref);
#if 0
    void* app_ref = ctx->app_interface->new(rl, new_sock);
#else
    void* app_ref = generic_app_new(rl, new_sock);
#endif
    if(error == 0) {
        List_add_back(ctx->connection_list, app_ref);
#if 0
        ctx->app_interface->run(app_ref, app_instance_done_cb, ctx);
#else
        generic_app_run(app_ref, app_instance_done_cb, ctx);
#endif
    } else{
        // termnate ?
    }
    /**
     * This is the place to decide if the thread is working hard enough and should is accept another connection.
     * In this simple application the answer is always yes. SO
     */
    runloop_post(rl, postable_start, ctx); 
}
static void app_instance_done_cb(void* app, void* server, int error)
{
    ServerCtxRef ctx = server;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    ListIterator itr = List_find(ctx->connection_list, app);
    assert(itr != NULL);
    List_itr_remove(ctx->connection_list, &itr);
#if 0
    ctx->app_interface->free(app);
#else
    generic_app_free(app);
#endif
}
int local_create_bound_socket(int port, const char *host)
{
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    socket_handle_t tmp_socket;
    server.sin_family = AF_INET; // or AF_INET6 (address family)
    server.sin_port = port;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if((tmp_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEPORT  , &yes, sizeof(yes))) != 0) {
        printf("setsockopt call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR  , &yes, sizeof(yes))) != 0) {
        printf("setsockopt call failed with errno %d \n", errno);
        assert(0);
    }
    if((result = bind(tmp_socket, (struct sockaddr *) &server, sizeof(server))) != 0) {
        printf("bind call failed with errno %d \n", errno);
        assert(0);
    }
//    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
//        printf("listen call failed with errno %d \n", errno);
//        assert(0);
//    }
    return tmp_socket;
}
