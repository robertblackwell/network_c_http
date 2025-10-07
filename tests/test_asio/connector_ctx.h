#ifndef c_http_tests_test_reactor_connector_h
#define c_http_tests_test_reactor_connector_h

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
#include <src/common/utils.h>
#include <src/common/socket_functions.h>
#include <src/runloop/runloop.h>



typedef struct Connector {
    int count;
    int max_count;
    int listen_fd;
    // TestAsyncServerRef servers[2];
}Connector, *ConnectorRef;

void* connector_thread_func(void* arg);

#endif