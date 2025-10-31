#ifndef C_HTTP_DEMO_ASYNC_PROCESS_MAIN_H
#define C_HTTP_DEMO_ASYNC_PROCESS_MAIN_H
#ifdef __cplusplus
extern "C" {
#endif

void process_main(char* host, int port, int nbr_threads, int nbr_connections_per_thread, int nbr_rountrips_per_connection);
#ifdef __cplusplus
}
#endif

#endif //C_HTTP_DEMO_ASYNC_PROCESS_MAIN_H
