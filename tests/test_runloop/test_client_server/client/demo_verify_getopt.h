#ifndef C_HTTP_Demo_VERIFY_GETOPT_H
#define C_HTTP_Demo_VERIFY_GETOPT_H
void verify_process_args(int argc, char* argv[], char** host_ip_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p);
void verify_usage();

#endif //C_HTTP_Demo_VERIFY_GETOPT_H
