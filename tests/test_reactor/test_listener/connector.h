#ifndef c_http_tests_test_reactor_connector_h
#define c_http_tests_test_reactor_connector_h
#define _GNU_SOURCE
#define XR_TRACE_ENABLE
//#include <http_in_c/async-old/types.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <http_in_c/unittest.h>
#include <http_in_c/common/utils.h>
#include <http_in_c/socket_functions.h>
#include <http_in_c/sync/sync_client.h>
#include <http_in_c/runloop/runloop.h>



typedef struct Connector {
    int count;
    int max_count;
    int listen_fd;
    // TestAsyncServerRef servers[2];
}Connector, *ConnectorRef;

void* connector_thread_func(void* arg);

#endif