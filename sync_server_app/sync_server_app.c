
#include <http_in_c/sync/sync.h>
#include <http_in_c/sync/sync_handler_example.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>
#include <unistd.h>

#define ENABLE_LOGX
#include <http_in_c/logger.h>
static void usage();
sync_server_r g_sref;

void sig_handler(int signo)
{
    printf("sync_app.c signal handler \n");
    if (signo == SIGINT) {
        printf("received SIGINT\n");
        sync_server_terminate(g_sref);
    }
}

int main(int argc, char* argv[])
{

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    int c;
    int port_number = 9001;
    int nbr_threads = 25;
    int read_buffer_size = 1000;

    opterr = 0;
    while((c = getopt(argc, argv, "p:t:")) != -1)
    {
        switch(c) {
            case 'p':
                LOG_FMT("-p options %s", optarg);
                port_number = atoi(optarg);
                break;
            case 't':
                LOG_FMT("-t options %s", optarg);
                nbr_threads = atoi(optarg);
                break;
            case '?':
                printf("There was an error in the options list\n");
            case 'h':
                usage();
                exit(0);
        }

    }

    printf("synchronous server threads: %d port: %d\n", nbr_threads, port_number);
    sync_server_r sref = sync_server_new(port_number, 1000, nbr_threads, app_handler_example);
    g_sref = sref;
    sync_server_listen(sref);
    sync_server_dispose(&sref);

}

static void usage()
{
    printf("Name: sync_server\n");
    printf("\nDescription\n");
    printf("\tsync_server is a multi-threaded http server. \n");
    printf("\tIt only responds to GET requests with one of the following 'target' values:\n\n");
    printf("\t-\t'/echo'\tthe response body contains a serialized versions of the request.\n");
    printf("\t-\t'/file'\treads a static html file from disk and sends it as a reply\n");
    printf("\t-\t'else' \teverything else is an error and gets a 404-page\n\n");
    printf("\tThe server can serve multiple requests on the same connection.\n");
    printf("\twill respect the 'CONNECTION: close/keep-alive header\n");
    printf("\twhen given a CONNECTION: keep-alive will hold the connection open forever\n");
    printf("\tit is up to the requester to either send CONNECTION: close or close the connection.\n");
    printf("\nOptions\n");

    printf("\t-h\tPrints this usage message. Does not take an argument\n");
    printf("\t-p\tRequired - Port number to use\n");
    printf("\t-t\tRequired - Number of simultaneous threads\n");

}