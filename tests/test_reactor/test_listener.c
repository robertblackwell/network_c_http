#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include <c_http/aio_api/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <c_http/unittest.h>
#include <c_http/dsl/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/api/client.h>
#include <c_http/runloop/reactor.h>
#include <c_http/runloop/watcher.h>
#include <c_http/runloop/w_timer.h>
#include <c_http/runloop/w_socket.h>
#include <c_http/runloop/w_listener.h>
#include <c_http/runloop/w_fdevent.h>

typedef int socket_handle_t;

struct TestServer_s {
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    XrReactorRef            reactor_ref;
    WListenerRef           listening_watcher_ref;
    XrConnListRef           conn_list_ref;
    int                     listen_counter;
    int                     accept_count;
};
typedef struct  TestServer_s TestServer, *TestServerRef;


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

    // sin.sin_len = sizeof(sin);
    if((result = setsockopt(tmp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) != 0) {
        goto error_02;
    }

    if((result = bind(tmp_socket, (struct sockaddr *) &sin, sizeof(sin))) != 0) {
        goto error_03;
    }

    if((result = listen(tmp_socket, SOMAXCONN)) != 0) {
        goto error_04;
    }
    return tmp_socket;

    error_01:
    printf("socket call failed with errno %d \n", errno);
    assert(0);
    error_02:
    printf("setsockopt call failed with errno %d \n", errno);
    assert(0);
    error_03:
    printf("bind call failed with errno %d \n", errno);
    assert(0);
    error_04:
    printf("listen call failed with errno %d \n", errno);
    assert(0);
}

void set_non_blocking(socket_handle_t socket)
{
    //
    // Ensure socket is in blocking mode
    //
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int fres = fcntl(socket, F_SETFL, modFlags2);
    assert(fres == 0);
}

static void on_event_listening(WListenerRef listener_ref, void *arg, uint64_t event)
{
//    assert(iobuf != NULL);
//    assert(conn_ref->handler_ref != NULL);
//    XrConn_write(conn_ref, iobuf, &on_handler_write, arg);

    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    TestServerRef server_ref = arg;
    int sock2 = accept(server_ref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        int errno_saved = errno;
        LOG_FMT("%s %d %d %s", "Listener thread :: accept failed terminating sock2 : ", sock2, errno, strerror(errno_saved));
    } else {
        printf("Sock2 successfull sock: %d server_ref %p listen_ref: %p  listen_count: %d\n", sock2, server_ref, listener_ref, server_ref->listen_counter);
        if(server_ref->listen_counter == 0) {
            sleep(1);
        }
        server_ref->listen_counter++;
        sleep(0.6);
    }
    close(sock2);
    printf("on_event_listen new socket is : %d\n", sock2);
}

static TestServerRef TestServer_new(int listen_fd)
{
    TestServerRef sref = malloc(sizeof(TestServer));
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;
    printf("TestServer_new %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}
static TestServerRef TestServer_init(TestServerRef sref, int listen_fd)
{
    sref->listening_socket_fd = listen_fd;
    sref->listen_counter = 0;
    sref->accept_count = 0;

    printf("TestServer_init %p   listen fd: %d\n", sref, listen_fd);
    return sref;
}

static void TestServer_dispose(TestServerRef *sref)
{
    ASSERT_NOT_NULL(*sref);
    free(*sref);
    *sref = NULL;
}
static void TestServer_listen(TestServerRef sref)
{
    ASSERT_NOT_NULL(sref)
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);
    sref->reactor_ref = XrReactor_new();
    sref->listening_watcher_ref = WListener_new(sref->reactor_ref, sref->listening_socket_fd);
    WListenerRef lw = sref->listening_watcher_ref;

    WListener_register(lw, on_event_listening, sref);
    // every time I try to call EOPLL_MOD is get an invalid arg error - though it works if I only use register EPOLL_ADD
//    WListener_arm(lw, on_event_listening, sref);
    printf("TestServer_listen reactor: %p listen sock: %d  lw: %p\n", sref->reactor_ref, sref->listening_socket_fd, lw);

    XrReactor_run(sref->reactor_ref, -1);
//    close(lw->fd);
}


//
// Tests fdevent
// start two threads
//  thread A waits on a fdevent object
//  thread B is a repeating timer, each timer tick it fires the fdevent object and after 5 ticks it terminates both event sources
//

typedef struct TestCtx_s  {
    int                 counter;
    int                 max_count;
    struct timespec     start_time;
    XrReactorRef        reactor;
    WTimerRef   timer;
    WFdEventRef        fdevent;
    int                 fdevent_counter;
} TestCtx;

TestCtx* TestCtx_new(int counter_init, int counter_max)
{
    TestCtx* tmp = malloc(sizeof(TestCtx));
    tmp->counter = counter_init;
    tmp->max_count = counter_max;
    tmp->fdevent_counter = 0;
    return tmp;
}

void* listener_thread_func(void* arg)
{
    TestServerRef server_ref = (TestServerRef) arg;
    printf("Listener thread server: %p \n", server_ref);
    TestServer_listen(server_ref);
}
typedef struct TestClient {
    int count;
    int max_count;
    int listen_fd;
    TestServerRef servers[2];
}TestClient, *TestClientRef;

void* connector_thread_func(void* arg)
{
    TestClient* tc = (TestClient*)arg;
    for(int i = 0; i < tc->max_count; i++) {
//        sleep(1);
        printf("Client about to connect %d \n", i);
        ClientRef client_ref = Client_new();
        Client_connect(client_ref, "localhost", 9001);
        sleep(0.2);
        Client_dispose(&client_ref);
    }
    // now wait here for all connections to be processed
    sleep(1);
    for(int i = 0; i < 2; i++) {
        TestServerRef server = tc->servers[i];
        WListenerRef listener = server->listening_watcher_ref;
        WListener_deregister(listener);
    }
}
int test_listeners()
{
    pthread_t listener_thread_1;
    pthread_t listener_thread_2;
    pthread_t connector_thread;
    TestClient tclient;
    tclient.max_count = 5;
    tclient.count = 0;
    int fd = create_listener_socket(9001, "localhost");
    tclient.listen_fd = fd;
    TestServerRef server1 = TestServer_new(fd);
    TestServerRef server2 = TestServer_new(fd);
    tclient.servers[0] = server1;
    tclient.servers[1] = server2;

    printf("Sizeof \n");
    set_non_blocking(fd);
    int r1 = pthread_create(&listener_thread_1, NULL, &listener_thread_func, server1);
    int r2 = pthread_create(&listener_thread_2, NULL, &listener_thread_func, server2);
    int r3 = pthread_create(&connector_thread, NULL, &connector_thread_func, &tclient);

    pthread_join(connector_thread, NULL);
    pthread_join(listener_thread_1, NULL);
    pthread_join(listener_thread_2, NULL);

    // all the connects were received and only those
    UT_EQUAL_INT((server1->listen_counter + server2->listen_counter), tclient.max_count);
    // each server thread got some of the connects
    UT_NOT_EQUAL_INT(server1->listen_counter, 0);
    UT_NOT_EQUAL_INT(server2->listen_counter, 0);

    return 0;
}
int main()
{
    UT_ADD(test_listeners);
    int rc = UT_RUN();
    return rc;
}
