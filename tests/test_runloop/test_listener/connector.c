
#define XR_TRACE_ENABLE
#include "connector.h"
//#include <http_in_c/async/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <rbl/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/sync/sync_client.h>

void* connector_thread_func(void* arg)
{
    Connector* tc = (Connector*)arg;
    for(int i = 0; i < tc->max_count; i++) {
//        sleep(1);
        printf("Client about to connect %d \n", i);
        sync_client_t* client_ptr = sync_client_new(10000);
        sync_client_connect(client_ptr, "localhost", 9001);
        usleep(200000);
        sync_client_dispose(&client_ptr);
    }
    printf("Connector loop ended \n");
    // now wait here for all connections to be processed
    sleep(1);
    // for(int i = 0; i < 2; i++) {
    //     TestAsyncServerRef server = tc->servers[i];
    //     RunloopListenerRef listener = server->listening_watcher_ref;
    //     runloop_listener_deregister(listener);
    // }
}
