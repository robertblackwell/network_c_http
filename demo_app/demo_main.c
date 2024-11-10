#include <http_in_c/demo_protocol/demo_server.h>
#include <http_in_c/http/message.h>
#include <http_in_c/common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <mcheck.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include "demo_process_main.h"

static void usage();
static void process_args(int argc, char* argv[], int* port, int* t, int* n);
void* thread_function(void* arg);
DemoServerRef g_sref;
void sig_handler(int signo)
{
    printf("demo_app.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        DemoServer_free(g_sref);
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
    char *host = "127.0.0.1";
    int nbr_threads = 2;
    int nbr_processes = 3;
    int nbr_connections_per_thread = 2;
    int nbr_roundtrips_per_connection = 2;
//    process_args(argc, argv, &port, &t, &n);
//    g_sref = sref;
    int child_pid;
    for (int p = 0; p < nbr_processes; p++) {
        if ((child_pid = fork()) == 0) {
            demo_process_main(host, port, nbr_threads, nbr_connections_per_thread, nbr_roundtrips_per_connection);
            exit(0);
        }
    }
    while (wait(NULL) > 0);
}
static void usage()
{
    printf("Name: demo_server\n");
    printf("\nDescription\n");
    printf("\tThis is a multi-threaded async_server waiting to receive STX.....ETX messages. \n");
    printf("\tIt respponds by sending back the request message.\n\n");
    printf("\tThe server can serve multiple requests on the same connection.\n");
    printf("\tDoes not support pipelining\n");
    printf("\tIt is up to the requester to either send CONNECTION: close or close the connection\n");
    printf("\tin order to close a session.\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-p\tRequired - Port number to use\n");
    printf("\t-t\tRequired - Number of simultaneous threads\n");
    printf("\t-n\tRequired - Number of simultaneous processes\n");

}
static void process_args(int argc, char* argv[], int* port, int* t, int* n) {
    int c;
    int port_number = 9001;
#ifdef CH_ASYNC_SINGLE_THREAD
#else
    int nbr_threads = 5;
#endif
    while ((c = getopt(argc, argv, "p:t:")) != -1) {
        switch (c) {
            case 'p':
                RBL_LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
#ifdef CH_ASYNC_SINGLE_THREAD
#else
                case 't':
                    RBL_LOG_FMT("-t options %s", optarg);
                    nbr_threads = atoi(optarg);
                    break;
#endif
            case '?':
                printf("There was an error in the options list\n");
            case 'h':
            default:
                usage();
                exit(0);
        }
    }
}
