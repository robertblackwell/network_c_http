#define _GNU_SOURCE
#include <c_http/sync/sync.h>
#include <c_http/sync/sync_handler_example.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>
#include <unistd.h>

#define ENABLE_LOG
#include <c_http/logger.h>

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
    int nbr_threads = 5;
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
        }

    }

    printf("Hello this is main \n");
    sync_server_r sref = sync_server_new(port_number, 1000, nbr_threads, app_handler_example);
    g_sref = sref;
    sync_server_listen(sref);
    sync_server_dispose(&sref);

}

