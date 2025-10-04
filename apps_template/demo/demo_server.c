#include <src/demo_protocol/demo_server.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <rbl/logger.h>
#include <rbl/check_tag.h>
#include <src/common/alloc.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include "tmpl_handler.h"

static DemoHandlerRef my_only_client;
static void set_non_blocking(socket_handle_t socket);
void on_event_listening(RunloopRef rl, void* arg_server_ref);

static void on_handler_completion_cb(void* void_server_ref, DemoHandlerRef handler_ref)
{
    RBL_LOG_FMT("file: demo_server.c on_handler_completion_cb \n");
    DemoServerRef server_ref = void_server_ref;
    RBL_CHECK_TAG(DemoServer_TAG, server_ref)
    RBL_CHECK_END_TAG(DemoServer_TAG, server_ref)
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    /**
     * remove the departing handle from the handler_list
     */
    ListIter x = List_find(server_ref->handler_list, handler_ref);
    assert(x != NULL);
    DemoHandlerRef href = List_itr_unpack(server_ref->handler_list, x);
    assert(href == handler_ref);
    /**
     * BEWARE - the list remove call also free()'s the handle object
     *
     * For the moment this fixed the following TODO
     */
    List_itr_remove(server_ref->handler_list, &x);
    RBL_CHECK_TAG(DemoHandler_TAG, handler_ref)
    demo_handler_free(handler_ref);

    /**
     * @TODO - this needs fixing - FIXED pass NULL as dispose function in demo_server_init
     *
    demo_handler_free(handler_ref);
    */
}
void demo_server_init(DemoServerRef sref, int port, char const * host, int listen_fd, DemoProcessRequestFunction process_request)
{
    RBL_SET_TAG(DemoServer_TAG, sref)
    RBL_SET_END_TAG(DemoServer_TAG, sref)
    sref->port = port;
    sref->host = host;
    socket_set_non_blocking(listen_fd);
    sref->listening_socket_fd = listen_fd;
    sref->process_request_function = process_request;

    sref->runloop_ref = runloop_new();
    sref->listening_watcher_ref = runloop_listener_new(sref->runloop_ref, sref->listening_socket_fd);
    sref->handler_list = List_new();
}
DemoServerRef demo_server_new(int port, char const * host, int listen_fd, DemoProcessRequestFunction* process_request)
{
    DemoServerRef sref = (DemoServerRef) eg_alloc(sizeof(DemoServer));
    demo_server_init(sref, port, host, listen_fd, process_request);
    return sref;
}
void demo_server_free(DemoServerRef this)
{
    RBL_CHECK_TAG(DemoServer_TAG, this)
    RBL_CHECK_END_TAG(DemoServer_TAG, this)
    ASSERT_NOT_NULL(this);
    // I own the listener
    runloop_listener_deregister(this->listening_watcher_ref);
    runloop_listener_free(this->listening_watcher_ref);
    // I own the runloop
    runloop_free(this->runloop_ref);
    // this should already be closed in the listener_deregister
    close(this->listening_socket_fd);
    // I own the handler list and it should be empty
    assert(List_size(this->handler_list) ==0);
    List_free(this->handler_list);
    free(this);
}

void demo_server_listen(DemoServerRef sref)
{
    RBL_CHECK_TAG(DemoServer_TAG, sref)
    RBL_CHECK_END_TAG(DemoServer_TAG, sref)
    ASSERT_NOT_NULL(sref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    RunloopListenerRef lw = sref->listening_watcher_ref;
    runloop_listener_register(lw, on_event_listening, sref);
    runloop_run(sref->runloop_ref, -1);
    RBL_LOG_FMT("DemoServer finishing");
}

void demo_server_terminate(DemoServerRef this)
{
    RBL_CHECK_TAG(DemoServer_TAG, this)
    RBL_CHECK_END_TAG(DemoServer_TAG, this)

    close(this->listening_socket_fd);
}
void on_event_listening(RunloopRef rl, void* arg_server_ref) // RunloopListenerRef listener_watcher_ref, uint64_t event)
{
    DemoServerRef server_ref = arg_server_ref; //listener_watcher_ref->listen_postable_arg;
    RBL_CHECK_TAG(DemoServer_TAG, server_ref)
    RBL_CHECK_END_TAG(DemoServer_TAG, server_ref)

    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        printf("accpt failed errno %d  sttrerror: %s\n", errno, strerror(errno));
        RBL_LOG_FMT("%s %d", "Listener thread :: accept failed terminating sock2 : ", sock2);
    }
    RBL_LOG_FMT("Listerner accepted sock fd: %d", sock2);
    DemoHandlerRef handler = demo_handler_new(
            rl,
            sock2,
            server_ref->process_request_function,
            on_handler_completion_cb,
            server_ref);

    List_add_back(server_ref->handler_list, handler);
}

