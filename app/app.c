#include <c_http/saved/sync_server.h>
#include <c_http/sync/sync_handler_example.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>
#include <unistd.h>

#define ENABLE_LOG
#include <c_http/logger.h>

SyncServerRef g_sref;

void sig_handler(int signo)
{
    printf("app.c signal handler \n");
    if (signo == SIGINT) {
        printf("received SIGINT\n");
        SyncServer_terminate( g_sref);
    }
}

int main(int argc, char* argv[])
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("app.c main signal() failed");
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
    SyncServerRef sref = SyncServer_new(port_number, nbr_threads, handler_example);
    g_sref = sref;
    SyncServer_listen(sref);
    SyncServer_dispose(&sref);

}

