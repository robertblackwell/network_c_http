#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include <c_http/xr/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <math.h>
#include <c_http/list.h>
#include <c_http/operation.h>
#include <c_http/oprlist.h>
#include <c_http/unittest.h>
#include <c_http/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/watcher.h>
#include <c_http/xr/timer_watcher.h>
#include <c_http/xr/socket_watcher.h>
#include <c_http/xr/listener.h>
#include <c_http/xr/fdevent.h>

typedef int socket_handle_t;

struct TestServer_s {
    socket_handle_t         listening_socket_fd;
    XrHandlerFunction       handler;
    XrReactorRef            reactor_ref;
    XrListenerRef           listening_watcher_ref;
    XrConnListRef           conn_list_ref;
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

static void on_event_listening(XrWatcherRef wp, void *arg, uint64_t event)
{
    XrSocketWatcherRef sw = (XrSocketWatcherRef)wp;
//    assert(iobuf != NULL);
//    assert(conn_ref->handler_ref != NULL);
//    XrConn_write(conn_ref, iobuf, &on_handler_write, arg);

    printf("listening_hander \n");
    struct sockaddr_in peername;
    unsigned int addr_length = (unsigned int) sizeof(peername);

    TestServerRef sref = arg;
    int sock2 = accept(sref->listening_socket_fd, (struct sockaddr *) &peername, &addr_length);
    if(sock2 <= 0) {
        LOG_FMT("%s %d %d", "Listener thread :: accept failed terminating sock2 : ", sock2, errno);
    } else {
        printf("Sock2 successfull %d \n", sock2);
    }
    close(sock2);
    printf("on_event_listen new socket is : %d\n", sock2);
}

static TestServerRef TestServer_new(int listen_fd)
{
    TestServerRef sref = (TestServerRef) eg_alloc(sizeof(TestServer));
    sref->listening_socket_fd = listen_fd;
    return sref;
}

static void TestServer_free(TestServerRef *sref)
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
    sref->listening_watcher_ref = XrListener_new(sref->reactor_ref, sref->listening_socket_fd);
    XrListenerRef lw = sref->listening_watcher_ref;
    XrListener_register(lw, on_event_listening, sref);
//    XrListener_arm(lw, on_event_listening, sref);
    printf("TestServer_listen reactor: %p listen sock: %d  lw: %p\n", sref->reactor_ref, sref->listening_socket_fd, lw);

    XrReactor_run(sref->reactor_ref, -1);
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
    XrTimerWatcherRef   timer;
    XrFdEventRef        fdevent;
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

//
// single repeating timer
//

//static int tw_counter_1 = 0;
static void callback_1(XrTimerWatcherRef watcher, void* ctx, XrTimerEvent event)
{
    uint64_t epollin = EPOLLIN & event;
    uint64_t error = EPOLLERR & event;
    TestCtx* ctx_p = (TestCtx*) ctx;
    XrFdEventRef fdev = ctx_p->fdevent;

    XR_TRACE("counter: %d event is : %lx  ", ctx_p->counter, event);
    if(ctx_p->counter >= ctx_p->max_count) {
        XR_TRACE_MSG(" clear timer");
        Xrtw_clear(watcher);
        XrFdEvent_deregister(fdev);
    } else {
        XrFdEvent_fire(fdev);
        ctx_p->counter++;
    }
}
void fdevent_handler(XrFdEventRef fdev_ref, void* arg, uint64_t ev_mask)
{
    TestCtx* t = (TestCtx*)arg;
    t->fdevent_counter++;
    XR_TRACE("w: %p arg: %p ev mask: %ld fdevent_counter % d", fdev_ref , arg, ev_mask, t->fdevent_counter);
}
int test_timer_single_repeating()
{
    // counter starts at 0 and increments to max 5
    TestCtx* test_ctx_p = TestCtx_new(0, 5);

    XrReactorRef rtor_ref = XrReactor_new();
    XrTimerWatcherRef tw_1 = Xrtw_new(rtor_ref, &callback_1, (void*)test_ctx_p, 1000, true);
    Xrtw_disarm(tw_1);
    XrFdEventRef fdev = XrFdEvent_new(rtor_ref);

    test_ctx_p->fdevent = fdev;
    test_ctx_p->timer = tw_1;


    XrFdEvent_register(fdev);
    XrFdEvent_arm(fdev, &fdevent_handler,test_ctx_p);
    Xrtw_rearm(tw_1);
    XrReactor_run(rtor_ref, 10000);

    // assert counter was increment correct number of times
    UT_EQUAL_INT(test_ctx_p->counter, test_ctx_p->max_count);
    UT_EQUAL_INT(test_ctx_p->fdevent_counter, test_ctx_p->max_count);
    free(test_ctx_p);
    XrReactor_free(rtor_ref);
    return 0;
}
int test_timer_multiple_repeating()
{
    TestCtx* test_ctx_p_1 = TestCtx_new(0, 5);
    TestCtx* test_ctx_p_2 = TestCtx_new(0, 6);

    XrReactorRef rtor_ref = XrReactor_new();

    XrTimerWatcherRef tw_1 = Xrtw_new(rtor_ref, &callback_1, test_ctx_p_1, 100, true);
    XrTimerWatcherRef tw_2 = Xrtw_new(rtor_ref, &callback_1, test_ctx_p_2, 100, true);

    XrReactor_run(rtor_ref, 10000);
    UT_EQUAL_INT(test_ctx_p_1->counter, test_ctx_p_1->max_count);
    UT_EQUAL_INT(test_ctx_p_2->counter, test_ctx_p_2->max_count);
    free(test_ctx_p_1);
    free(test_ctx_p_2);
    XrReactor_free(rtor_ref);
    return 0;
}

void* listener_thread_func(void* arg)
{
    int fd = (int)(long)arg;
    TestServerRef server = TestServer_new(fd);
    printf("Listener thread server: %p \n", server);
    TestServer_listen(server);
}
void* connector_thread_func(void* arg)
{
    sleep(100);
}
int test_listeners()
{
    pthread_t listener_thread_1;
    pthread_t listener_thread_2;
    pthread_t connector_thread;
    int fd = create_listener_socket(9001, "localhost");
    set_non_blocking(fd);
    int r1 = pthread_create(&listener_thread_1, NULL, &listener_thread_func, (void*)(long)fd);
    int r2 = pthread_create(&listener_thread_2, NULL, &listener_thread_func, (void*)(long)fd);
//    int r3 = pthread_create(&connector_thread, NULL, &connector_thread_func, (void*)(long)9001);

    pthread_join(listener_thread_1, NULL);
    pthread_join(listener_thread_2, NULL);
//    pthread_join(connector_thread, NULL);


    return 0;
}
int main()
{
    UT_ADD(test_listeners);
//    UT_ADD(test_timer_multiple_repeating);
    int rc = UT_RUN();
    return rc;
}
