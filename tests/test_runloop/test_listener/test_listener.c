
#define XR_TRACE_ENABLE
#include <stdio.h>
#include <pthread.h>
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
    listener_ctx_listen(server_ref);
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
    int fd = create_listener_socket(9001, "localhost");
    socket_set_non_blocking(fd);

    tclient.listen_fd = fd;

    ListenerCtxRef server1 = listener_ctx_new(1, fd);
    ListenerCtxRef server2 = listener_ctx_new(2, fd);
    printf("Sizeof \n");
    int r1 = pthread_create(&listener_thread_1, NULL, &listener_thread_func, server1);
    int r2 = pthread_create(&listener_thread_2, NULL, &listener_thread_func, server2);
    int r3 = pthread_create(&connector_thread, NULL, &connector_thread_func, &tclient);

    pthread_join(connector_thread, NULL);
    pthread_join(listener_thread_1, NULL);
    pthread_join(listener_thread_2, NULL);

    printf("test_listener all threads have joined \n");
    /**
     * Test that each listener got some of the connections and that
     * all connections were recorded
     */
    UT_EQUAL_INT((server1->listen_counter + server2->listen_counter), tclient.max_count);
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

