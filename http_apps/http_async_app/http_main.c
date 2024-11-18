#include <http_in_c/http_protocol/http_server.h>
#include <http_in_c/http/http_message.h>
#include <http_in_c/common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <getopt.h>
#include <mcheck.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include "http_process_main.h"

static void usage();
static void process_args(int argc, char* argv[], char** host_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p, int* nbr_processes_p);
void* thread_function(void* arg);
char* default_host = "127.0.0.1";
int   default_port = 9011;

HttpServerRef g_sref;
void sig_handler(int signo)
{
    printf("htp_app.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        HttpServer_free(g_sref);
        g_sref = NULL;
        exit(0);
    }
}
int main(int argc, char** argv) {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    //printf("Hello this is xr-junk main \n");
    int port = 9011;
    char* host = malloc(100);
    int nbr_threads = 2;
    int nbr_processes = 3;
    int nbr_connections_per_thread = 2;
    int nbr_roundtrips_per_connection = 2;
    int i = 0;
#if 0
    while(argv[i] != NULL) {
        printf("arg[%d] = %s\n", i, argv[i]);
        i++;
    }
#endif
    process_args(argc, argv,
                 &host,
                 &port,
                 &nbr_roundtrips_per_connection,
                 &nbr_connections_per_thread,
                 &nbr_threads,
                 &nbr_processes);
//    g_sref = sref;
    printf("host: %s port: %d nbr_processes: %d nbr_threads: %d \n",
           host, port, nbr_processes, nbr_threads
           );
    int child_pid;
    for (int p = 0; p < nbr_processes; p++) {
        if ((child_pid = fork()) == 0) {
            http_process_main(host, port, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
            exit(0);
        }
    }
    while (wait(NULL) > 0);
}
static void usage()
{
    printf("Name: http_server\n");
    printf("\nDescription\n");
    printf("\tThis is a multi-process multi-threaded async_server waiting to receive http 1.1 messages. \n");
    printf("\n");
    printf("\tThat is this program forks a number of child processes which in turn start a number of  \n");
    printf("\tthreads. Each thread in each subprocess runs an instance of the server. \n");
    printf("\n");
    printf("\tThe total number of server instance running is (-n value) * (-t value). \n");
    printf("\n");
    printf("\tEach server instance listen for request on the host:port options described below. \n");
    printf("\n");
    printf("\tEach server instance can serve multiple consecutive requests on the same connection.\n");
    printf("\n");
    printf("\tA server instance responds by sending back the request message.\n\n");
    printf("\n");
    printf("\tIt is up to the requester to either send CONNECTION: close or close the connection\n");
    printf("\tin order to close a session.\n");
    printf("\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-i\tHost ip address                                          - default 127.0.0.1\n");
    printf("\t-p\tPort number to use                                       - default 9011\n");
    printf("\t-n\tNbr Processes running servers                            - default 2\n");
    printf("\t-t\tNbr threads per process each running a server            - default 3\n");

}
static void process_args(int argc, char* argv[], char** host_ip_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p, int* nbr_processes_p) {
    int c;
    char* host_ptr = NULL;
    int port_number = 9011;
    int nbr_processes = 1;
    int nbr_threads = 1;
    int nbr_connections_per_thread = 1;
    int nbr_roundtrips_per_connection = 1;
    while ((c = getopt(argc, argv, "n:h:r:c:p:t:")) != -1) {
//        printf("c = %c\n", (char)c);
        switch (c) {
            case 'n':
                RBL_LOG_FMT("-t options %s", optarg);
                nbr_processes = atoi(optarg);
                break;
            case 't':
                RBL_LOG_FMT("-t options %s", optarg);
                nbr_threads = atoi(optarg);
                break;
            case 'r':
                RBL_LOG_FMT("-r options %s", optarg);
                nbr_roundtrips_per_connection = atoi(optarg);
                break;
            case 'c':
                RBL_LOG_FMT("-c options %s", optarg);
                nbr_connections_per_thread = atoi(optarg);
                break;
            case 'p':
                RBL_LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
            case 'h':
                RBL_LOG_FMT("-h options %s", optarg);
                int len = (int)strlen(optarg);
                host_ptr = malloc(len+1);
                strcpy(host_ptr, optarg);
//                port_number = atoi(optarg);
                break;
            case '?':
                printf("There was an error in the options list\n");
            default:
                usage();
                exit(0);
        }
    }
    if(host_ptr != NULL) *host_ip_p = host_ptr;
    *port = port_number;
    *nbr_roundtrips_per_connection_p = nbr_roundtrips_per_connection;
    *nbr_connections_per_thread_p = nbr_connections_per_thread;
    *nbr_threads_p = nbr_threads;
    *nbr_processes_p = nbr_processes;

}
