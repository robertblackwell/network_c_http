
#include <http_in_c/runloop/rl_internal.h>
#include <http_in_c/demo_protocol/demo_server.h>
#include <http_in_c/demo_protocol/demo_handler.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <rbl/check_tag.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/socket_functions.h>

static DemoHandlerRef my_only_client;

static socket_handle_t create_listener_socket(int port, const char *host);
static void set_non_blocking(socket_handle_t socket);
void on_event_listening(RunloopListenerRef listener_watcher_ref, uint64_t event);

static void on_handler_completion_cb(void* void_server_ref, DemoHandlerRef handler_ref)
{
    printf("file: demo_server.c on_handler_completeion_cb \n");

    DemoServerRef server_ref = void_server_ref;
    RBL_CHECK_TAG(DemoServer_TAG, server_ref)
    RBL_CHECK_END_TAG(DemoServer_TAG, server_ref)
    ListIter x = List_find(server_ref->handler_list, handler_ref);
    DemoHandlerRef href = List_itr_unpack(server_ref->handler_list, x);
    List_itr_remove(server_ref->handler_list, &x);
}
DemoServerRef DemoServer_new(int port)
{
    DemoServerRef sref = (DemoServerRef) eg_alloc(sizeof(DemoServer));
    RBL_SET_TAG(DemoServer_TAG, sref)
    RBL_SET_END_TAG(DemoServer_TAG, sref)
    sref->port = port;
    sref->runloop_ref = runloop_new();
    sref->listening_socket_fd = create_listener_socket(port, "127.0.0.1");
    set_non_blocking(sref->listening_socket_fd);
    sref->listening_watcher_ref = runloop_listener_new(sref->runloop_ref, sref->listening_socket_fd);
    // TODO this is a memory leak
    // the list is the owner of handler references
    sref->handler_list = List_new(demohandler_anonymous_dispose);
//    sref->completion_callback = &on_handler_completion_cb;
    return sref;
}
void DemoServer_free(DemoServerRef this)
{
    RBL_CHECK_TAG(DemoServer_TAG, this)
    RBL_CHECK_END_TAG(DemoServer_TAG, this)
    ASSERT_NOT_NULL(this);
    runloop_listener_deregister(this->listening_watcher_ref);
    runloop_listener_free(this->listening_watcher_ref);
    close(this->listening_socket_fd);
    runloop_free(this->runloop_ref);
    List_dispose(&(this->handler_list));
    free(this);

}
void DemoServer_dispose(DemoServerRef *sref)
{
    RBL_CHECK_TAG(DemoServer_TAG, *sref)
    RBL_CHECK_END_TAG(DemoServer_TAG, *sref)
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}

void DemoServer_listen(DemoServerRef sref)
{
    RBL_CHECK_TAG(DemoServer_TAG, sref)
    RBL_CHECK_END_TAG(DemoServer_TAG, sref)
    ASSERT_NOT_NULL(sref)
    int port = sref->port;
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    RunloopListenerRef lw = sref->listening_watcher_ref;
    runloop_listener_register(lw, on_event_listening, sref);
    runloop_run(sref->runloop_ref, -1);
    RBL_LOG_FMT("DemoServer finishing");

}

void DemoServer_terminate(DemoServerRef this)
{
    RBL_CHECK_TAG(DemoServer_TAG, this)
    RBL_CHECK_END_TAG(DemoServer_TAG, this)

    close(this->listening_socket_fd);
}

//
// create a listening socket from host and port
//
static socket_handle_t create_listener_socket(int port, const char *host)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socket_handle_t tmp_socket;
    sin.sin_family = AF_INET; // or AF_INET6 (address family)
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    int result;
    int yes = 1;

    if((tmp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        goto error_01;
    }
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0) {
        goto error_02;
    }
    if((result = bind(tmp_socket, (struct sockaddr *) &sin, sizeof(sin))) != 0) {
        int x = errno;
        goto error_03;
    }
    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
        goto error_04;
    }
    return tmp_socket;

    error_01:
    printf("socket call failed with errno %d %s\n", errno, strerror(errno));
    assert(0);
    error_02:
    printf("setsockopt call failed with errno %d %s\n", errno, strerror(errno));
    assert(0);
    error_03:
    printf("bind call failed with errno %d %s\n", errno, strerror(errno));
    assert(0);
    error_04:
    printf("listen call failed with errno %d %s\n", errno, strerror(errno));
    assert(0);
}

void set_non_blocking(socket_handle_t socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    assert(fres == 0);
}

void on_event_listening(RunloopListenerRef listener_watcher_ref, uint64_t event)
{

    printf("listening_hander \n");
    DemoServerRef server_ref = listener_watcher_ref->listen_postable_arg;
    RBL_CHECK_TAG(DemoServer_TAG, server_ref)

//    DemoServerRef server_ref = listener_watcher_ref->listen_arg;

    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        printf("accpt failed errno %d  sttrerror: %s\n", errno, strerror(errno));
        RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    RBL_LOG_FMT("Listerner accepted sock fd: %d", sock2);
    DemoHandlerRef handler = demohandler_new(
            sock2,
            runloop_listener_get_runloop(listener_watcher_ref),
            on_handler_completion_cb,
            server_ref);

    List_add_back(server_ref->handler_list, handler);
}

