#include <src/runloop/runloop.h>
// //#include <src/runloop/rl_internal.h>
#include "listener_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <rbl/logger.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>

#define L_STATE_EAGAIN 22
#define L_STATE_READY 33
#define L_STATE_INITIAL 44
#define L_STATE_STOPPED 55
#define L_STATE_ERROR 66

static void on_timer(RunloopRef rl, void* arg);
static void on_read_ready(RunloopRef rl, void* listener_ref_arg);
static void handle_new_socket(ListenerCtxRef ctx, int sock);
static bool can_do_more(ListenerCtxRef ctx);
static void try_accept(ListenerCtxRef ctx);
static void postable_try_accept(RunloopRef rl, void* arg);

ListenerCtxRef listener_ctx_new(int listen_fd, int id, RunloopRef rl)
{
    runloop_verify(rl);
    ListenerCtxRef sref = malloc(sizeof(TestServer));
    RBL_SET_TAG(Ctx_TAG, sref)
    RBL_SET_END_TAG(Ctx_TAG, sref)
    VERIFY_RUNLOOP(rl)
    sref->l_state = L_STATE_INITIAL;
    sref->listening_socket_fd = listen_fd;
    sref->listen_count = 0;
    sref->accept_count = 0;
    sref->id = id;
    sref->runloop_ref = rl;
    sref->rl_event = runloop_listener_new(rl, listen_fd);
    printf("listener_ctx_new %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}

void listener_ctx_init(ListenerCtxRef sref, int listen_fd, int id, RunloopRef rl)
{
    RBL_SET_TAG(Ctx_TAG, sref)
    RBL_SET_END_TAG(Ctx_TAG, sref)
    VERIFY_RUNLOOP(rl)
    runloop_verify(rl);
    sref->listening_socket_fd = listen_fd;
    sref->l_state = L_STATE_INITIAL;
    sref->listen_count = 0;
    sref->accept_count = 0;
    sref->id = id;
    sref->runloop_ref = rl;
    sref->rl_event = runloop_listener_new(rl, listen_fd);
    printf("listener_ctx_init %p   listen fd: %d\n", sref, listen_fd);
}

void listener_ctx_free(ListenerCtxRef sref)
{
    ASSERT_NOT_NULL(sref);
    RBL_CHECK_TAG(Ctx_TAG, sref)
    RBL_CHECK_END_TAG(Ctx_TAG, sref)
    free(sref);
}
void listener_ctx_run(ListenerCtxRef ctx)
{
    ASSERT_NOT_NULL(ctx)
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    RunloopEventRef rlevent = ctx->rl_event;

    // runloop_listener_register(rlevent, on_event_listening, ctx);

    ctx->timer_ref = runloop_timer_new(ctx->runloop_ref);
    runloop_timer_register(ctx->timer_ref, &on_timer, (void *)ctx, 5000000, false);
    runloop_post(ctx->runloop_ref, postable_try_accept, ctx);    
    runloop_run(ctx->runloop_ref, -1);
    printf("Listener runloop ended \n");
    runloop_free(ctx->runloop_ref);
}

static void postable_try_accept(RunloopRef rl, void* arg)
{
    ListenerCtxRef ctx = arg;
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    RunloopEventRef rlevent = ctx->rl_event;
    runloop_listener_register(rlevent, on_read_ready, ctx);
    int fd = runloop_listener_get_fd(rlevent);
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
static void handle_new_socket(ListenerCtxRef ctx, int sock)
{
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    int nbsock = socket_set_blocking(sock);
    RBL_LOG_FMT("handle_new_socket ctx: %p sock: %d accept_count: %d", ctx, sock, ctx->accept_count)
    ctx->accept_count++;
    char buf[300];
    int nr = read(nbsock, buf, 100);
    buf[nr] = '\0';
    RBL_LOG_FMT("handle_new_socket from client: %s ", buf);
    char wbuf[300];
    int ln = snprintf(wbuf, 100, "Reply from server: %s", buf);
    int wr = write(nbsock, wbuf, ln);
    close(sock);
    // if(ctx->accept_count >= ctx->max_accept_count) {
    //     ctx->l_state = L_STATE_STOPPED;
    // }
}
static bool can_do_more(ListenerCtxRef ctx)
{
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    return true;
}
static void try_accept(ListenerCtxRef ctx)
{
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    RunloopEventRef rlevent = ctx->rl_event;
    RunloopRef rl = runloop_listener_get_runloop(rlevent);
    int fd = runloop_listener_get_fd(rlevent);
    RBL_LOG_FMT("try_accept fd: %d ", fd)
    int sock2 = accept(fd, (struct sockaddr *) &peername, &addr_length);
    int errno_saved = errno;
    if(sock2 > 0) {
        ctx->listen_count++;
        ctx->accept_count++;
        ctx->l_state = L_STATE_READY;
        handle_new_socket(ctx, sock2);
        if(can_do_more(ctx)) {
            runloop_post(rl, postable_try_accept, ctx);
        }
    } else if (sock2 == 0) {
       // listen socket is closed ?? 
    } else {
        if(errno_saved == EAGAIN) {
            RBL_LOG_FMT("try_accept fd: %d EAGAIN", fd)
            ctx->l_state = L_STATE_EAGAIN;
            runloop_listener_arm(rlevent, on_read_ready, ctx);
        } else {
            ctx->l_state = L_STATE_ERROR;
        }
    }

}
static void on_read_ready(RunloopRef rl, void* arg)
{
    ListenerCtxRef ctx = arg;
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    RunloopEventRef rlevent = ctx->rl_event;
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
    ListenerCtxRef ctx = (ListenerCtxRef) listener_ref_arg;
    RBL_CHECK_TAG(Ctx_TAG, ctx)
    RBL_CHECK_END_TAG(Ctx_TAG, ctx)
    RBL_LOG_MSG("on_timer closing runloop ");
    runloop_close(ctx->runloop_ref);
//    runloop_free(listener_ref->reactor_ref);
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
