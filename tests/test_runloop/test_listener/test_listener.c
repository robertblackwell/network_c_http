
#define XR_TRACE_ENABLE
#include <stdio.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <string.h>
#include <rbl/unittest.h>
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include "listener_ctx.h"
#include "connector_ctx.h"

/**
 * This test has two goals:
 * 1.   verify that a listener event is handled correcly. That is a listen event on a socket is generated
 *      when another thread connects to the same host/port.
 *
 * 2.   In addition we are testing that multiple threads can be listening on the same socket and that
 *      a connect to that host/port will only cause a listen event on one of the listening threads.
 *
 *      This is of particular interest as it allows a multi-threaded server to operate without additional
 *      listener thread and without synchronization between the listener and the thread that services the connection.
 */

void* listener_thread_func(void* arg)
{
    ListenerCtxRef server_ref = (ListenerCtxRef) arg;
    printf("Listener thread server: %p \n", server_ref);
    listener_ctx_run(server_ref);
    return NULL;
}

int test_listeners()
{
    pthread_t listener_thread_1;
    pthread_t listener_thread_2;
    pthread_t connector_thread;
    Connector tclient;
    tclient.max_count = 5;
    tclient.count = 0;
    /**
     * NOTE: the creation of the socket file descriptor that will be used for listen()-ing and accept()
     *
     * Further the socket is created and bound to a host/port in the main thread and passed to the other
     * threads.
     * Need to test whether this is necessary
     *  -   NO works if works if created in each thread - see conditional
     *      compile below
     *
     * Also need to test it for forking processes. Aslo works for forking processes. See
     * demo_sync and demo_async
     *
     * Finally can I do this inside the runloop_listener_init() function - YES
     * again see the conditional compiles below
     *
     * */
    int port = 9002;
    int fd = local_create_bound_socket(port, "localhost");
    socket_set_non_blocking(fd);

    tclient.listen_fd = fd;
    tclient.port = port;

    ListenerCtxRef server1 = listener_ctx_new(fd, 1, runloop_new());
    // ListenerCtxRef server2 = listener_ctx_new(fd, 2, runloop_new());
    printf("Sizeof \n");
    int r1 = pthread_create(&listener_thread_1, NULL, &listener_thread_func, server1);
    // int r2 = pthread_create(&listener_thread_2, NULL, &listener_thread_func, server2);
    sleep(2);
    // int r3 = pthread_create(&connector_thread, NULL, &connector_thread_func, &tclient);

    // pthread_join(connector_thread, NULL);
    pthread_join(listener_thread_1, NULL);
    // pthread_join(listener_thread_2, NULL);
    printf("test_listener all threads hace joined \n");
    /**
     * Test that each listener got some of the connections and that
     * all connections were recorded
     */
    printf("client max_count: %d successful connects : %d \n", tclient.max_count, tclient.count);
    printf("server 1 listen_count: %d accept_count: %d \n", server1->listen_count, server1->accept_count);
    // printf("server 2 listen_count: %d accept_count: %d \n", server2->listen_count, server2->accept_count);
    // UT_EQUAL_INT((server1->listen_count + server2->listen_count), tclient.max_count);
    // UT_NOT_EQUAL_INT(server1->listen_count, 0);
    // UT_NOT_EQUAL_INT(server2->listen_count, 0);

    return 0;
}
int main()
{
    UT_ADD(test_listeners);
    int rc = UT_RUN();
    return rc;
}

