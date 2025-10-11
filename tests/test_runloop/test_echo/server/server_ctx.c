#include <src/runloop/runloop.h>
// //#include <src/runloop/rl_internal.h>
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
#include <echo_app/echo_app.h>
#include "server_ctx.h"

#define L_STATE_EAGAIN 22
#define L_STATE_READY 33
#define L_STATE_INITIAL 44
#define L_STATE_STOPPED 55
#define L_STATE_ERROR 66

static void on_timer(RunloopRef rl, void* arg);
static void on_accept_ready(RunloopRef rl, void* listener_ref_arg);
static void handle_new_socket(void* server, int sock, int error);
static bool can_do_more(ServerCtxRef ctx);
static void try_accept(ServerCtxRef ctx);
static void postable_start(RunloopRef rl, void* arg);
static void handle_app_done(EchoAppRef app_ref, void* arg, int error);
static void app_instance_done_cb(void* app, void* server, int error);


ServerCtxRef server_ctx_new(RunloopRef rl, int listener_fd)
{
    ServerCtxRef sref = malloc(sizeof(ServerCtx));
    server_ctx_init(sref, rl, listener_fd);
    RBL_SET_TAG(ServerCtx_TAG, sref)
    RBL_SET_END_TAG(ServerCtx_TAG, sref)
    return sref;
}

void server_ctx_init(ServerCtxRef server_ctx, RunloopRef rl, int fd)
{
    RBL_SET_TAG(ServerCtx_TAG, server_ctx)
    RBL_SET_END_TAG(ServerCtx_TAG, server_ctx)
    server_ctx->runloop_ref = runloop_new();
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
#if 0
static void postable_try_accept(RunloopRef rl, void* arg)
{
    ServerCtxRef ctx = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    TcpListenerRef tcp_list = ctx->tcp_listener_ref;
    RunloopListenerRef rllistener_ref = tcp_list->rl_listener_ref;
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
#endif
#if 0    
static void postable_read_cb(RunloopRef rl, void* arg)
{

}
#endif
static void handle_new_socket(void* server, int new_sock, int error)
{
    ServerCtxRef ctx = server;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    // int nbsock = socket_set_blocking(sock);
    RBL_LOG_FMT("handle_new_socket ctx: %p sock: %d ", ctx, new_sock)
    RunloopRef rl = runloop_listener_get_runloop(ctx->tcp_listener_ref->rl_listener_ref);
    EchoAppRef app_ref = echo_app_new(rl, new_sock);
    if(error == 0) {
        List_add_back(ctx->connection_list, app_ref);
        echo_app_run(app_ref, app_instance_done_cb, ctx);
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

}
#if 0
static void handle_app_done(EchoAppRef app_ref, void* arg, int error)
{
    ServerCtxRef ctx = arg;
    RBL_CHECK_TAG(ServerCtx_TAG, ctx)
    RBL_CHECK_END_TAG(ServerCtx_TAG, ctx)
    ListIterator itr = List_find(ctx->connection_list, app_ref);
    assert(itr != NULL);
    List_itr_remove(ctx->connection_list, itr);
    /*
    * If the server was using some kinf od throttling mechaism the completion
    * of an app instance would invoke the load balance algorithm
    */
   /**
    * Here deallocate the resources associated with app_ref
    */
   echo_app_free(app_ref);

}

static bool can_do_more(ServerCtxRef ctx)
{
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    return true;
}
static void try_accept(ServerCtxRef ctx)
{
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    RunloopListenerRef rl_listener = ctx->rl_listener_ref;
    RunloopRef rl = runloop_listener_get_runloop(rl_listener);
    int fd = runloop_listener_get_fd(rl_listener);
    RBL_LOG_FMT("try_accept accept_fd: %d ", fd)
    int sock2 = accept(fd, (struct sockaddr *) &peername, &addr_length);
    int errno_saved = errno;
    if(sock2 > 0) {
        RBL_LOG_FMT("try_accept success sock2: %d", sock2)
        ctx->l_state = L_STATE_READY;
        handle_new_socket(ctx, sock2);
    } else if (sock2 == 0) {
       // listen socket is closed ?? 
       // terminate the runloop ? 
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_accept fd: %d EAGAIN", fd)
            ctx->l_state = L_STATE_EAGAIN;
            // make sure the runloop_listener is armed to catch the ready event
            // only on some pplatforms is ths necessary
            runloop_listener_arm(rl_listener, on_accept_ready, ctx);
        } else {
            ctx->l_state = L_STATE_ERROR;
            // terminate the runloop
        }
    }

}
static void on_accept_ready(RunloopRef rl, void* arg)
{
    ServerCtxRef ctx = arg;
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    RunloopListenerRef rl_listener = ctx->rl_listener_ref;
    RBL_LOG_FMT("on_read_ready fd: %d state: %d", runloop_listener_get_fd(rlevent), ctx->l_state)
    switch(ctx->l_state) {
        case L_STATE_EAGAIN:
            ctx->l_state = L_STATE_READY;
        case L_STATE_READY:
            try_accept(ctx);
            break;
        case L_STATE_INITIAL:
        case L_STATE_STOPPED:
        case L_STATE_ERROR:
            RBL_LOG_FMT("on_event_listen should not be here fd: %d state: %d", runloop_listener_get_fd(rlevent), ctx->l_state)
        default:
            assert(0);
            break;
    }
}
/**
 * When the timer fires it is time to kill the listener.
 */
static void on_timer(RunloopRef rl, void* listener_ref_arg)
{
    printf("on_timer entered \n");
    ServerCtxRef ctx = (ServerCtxRef) listener_ref_arg;
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    RBL_LOG_MSG("on_timer closing runloop ");
    runloop_close(ctx->runloop_ref);
//    runloop_free(listener_ref->reactor_ref);
}
#endif
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
