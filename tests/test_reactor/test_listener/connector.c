#define _GNU_SOURCE
#define XR_TRACE_ENABLE
#include "connector.h"
#include <c_http/async/types.h>
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
#include <c_http/common/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/sync/sync_client.h>

void* connector_thread_func(void* arg)
{
    Connector* tc = (Connector*)arg;
    for(int i = 0; i < tc->max_count; i++) {
//        sleep(1);
        printf("Client about to connect %d \n", i);
        ClientRef client_ref = Client_new();
        Client_connect(client_ref, "localhost", 9001);
        usleep(200000);
        Client_dispose(&client_ref);
    }
    printf("Connector loop ended \n");
    // now wait here for all connections to be processed
    sleep(1);
    // for(int i = 0; i < 2; i++) {
    //     TestAsyncServerRef server = tc->servers[i];
    //     RtorListenerRef listener = server->listening_watcher_ref;
    //     rtor_listener_deregister(listener);
    // }
}
