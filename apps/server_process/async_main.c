#include <apps/server/server_ctx.h>
#include <common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
// #include <mcheck.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include "async_process_main.h"
#ifdef __cplusplus
extern "C" {
#endif

static void usage();
static void process_args(int argc, char* argv[], char** host_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p, int* nbr_processes_p);
void* thread_function(void* arg);
char* default_host = "localhost";
int   default_port = 9002;

ServerCtxRef g_sref;
void sig_handler(int signo)
{
    printf("async_main.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        if(g_sref != NULL ) server_ctx_free(g_sref);
        g_sref = NULL;
        exit(0);
    }
}
int main(int argc, char** argv) {

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        int errno_saved = errno;
        printf("sync_app.c main line:%d signal() failed errno: %d %s\n", __LINE__, errno_saved, strerror(errno_saved));
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        int errno_saved = errno;
        printf("sync_app.c main line:%d signal() failed errno: %d %s\n", __LINE__, errno_saved, strerror(errno_saved));
    }
    //printf("Hello this is xr-junk main \n");
    int port;
    char* host = malloc(100);
    int nbr_threads;
    int nbr_processes;
    int nbr_connections_per_thread;
    int nbr_roundtrips_per_connection;
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
#define ONE_PROCESSX
#ifdef ONE_PROCESS
    process_main(host, port, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
#else

    int child_pid;
    for (int p = 0; p < nbr_processes; p++) {
        if ((child_pid = fork()) == 0) {
//            for (int k = 0; k < 5; k++) {
//                printf("child process %d\n", getpid());
//                sleep(2);
//            }
            process_main(host, port, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
//            exit(0);
        }
    }
    pid_t wpid;
    sleep(20);
    while ((wpid = wait(NULL)) > 0) {
        printf("Child process %d terminated \n", wpid);
    }
#endif
    printf("All processes finished\n");
}
static void usage()
{
    printf("Name: demo_server\n");
    printf("\nDescription\n");
    printf("\tThis is a multi-process multi-threaded async_server waiting to receive STX.....ETX messages. \n");
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
    printf("\t-i\tHost name/ip address                                     - default localhost\n");
    printf("\t-p\tPort number to use                                       - default 9002\n");
    printf("\t-n\tNbr Processes running servers                            - default 2\n");
    printf("\t-t\tNbr threads per process each running a server            - default 3\n");

}
static void process_args(int argc, char* argv[], char** host_ip_p, int* port, int* nbr_roundtrips_per_connection_p, int* nbr_connections_per_thread_p, int* nbr_threads_p, int* nbr_processes_p) {
    int opt_id;
    char* host_ptr = "localhost";
    int port_number = 9002;
    int nbr_processes = 1;
    int nbr_threads = 1;
    int nbr_connections_per_thread = 1;
    int nbr_roundtrips_per_connection = 100;
    while ((opt_id = getopt(argc, argv, "n:h:r:c:p:t:")) != -1) {
//        printf("c = %c\n", (char)c);
        switch (opt_id) {
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
#ifdef __cplusplus
}
#endif
