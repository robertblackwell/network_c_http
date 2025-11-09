#ifndef c_http_tests_test_reactor_connector_h
#define c_http_tests_test_reactor_connector_h

typedef struct Connector {
    int count;
    int max_count;
    int listen_fd;
    // TestAsyncServerRef servers[2];
}Connector, *ConnectorRef;

void* connector_thread_func(void* arg);

#endif