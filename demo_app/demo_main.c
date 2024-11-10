#include <http_in_c/demo_protocol/demo_server.h>
#include <http_in_c/http/message.h>
#include <http_in_c/common/socket_functions.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

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
#define MAX_NUMBER_THREADS 5
typedef struct thread_context_s {
    int             ident;
    pthread_t       thread;
    int             port;
    const char*     host;
    int             listening_socket;
    void*           return_value;
    DemoServerRef   server_ref;
} thread_context_t;

static thread_context_t thread_table[MAX_NUMBER_THREADS];

int main(int argc, char** argv)
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    //printf("Hello this is xr-junk main \n");
    int port = 9011;
    int t = 2;
    int n;
    char* host = "127.0.0.1";
//    process_args(argc, argv, &port, &t, &n);
//    g_sref = sref;
    int number_threads = t;
//    int listening_socket_fd = create_listener_socket(port, host);
    for(int i = 0; i < number_threads; i++) {
        thread_context_t* ctx = &(thread_table[i]);
        ctx->ident = i;
        ctx->port = port;
        ctx->host = host;
//        ctx->listening_socket = listening_socket_fd;
//        ctx->server_ref = DemoServer_new(port, host, listening_socket_fd, NULL);
        pthread_create(&(ctx->thread), NULL, thread_function, ctx);
    }
    for(int i = 0; i < number_threads; i++) {
        thread_context_t* ctx = &(thread_table[i]);
        pthread_join(ctx->thread, (void**)&(ctx->return_value));
    }
}
void* thread_function(void* arg)
{
    thread_context_t* ctx = arg;
    RBL_LOG_FMT("thread function ident: %d pthread_t: %lu listening_socket: %d", ctx->ident, ctx->thread, ctx->listening_socket)
    int listening_socket_fd = create_listener_socket(ctx->port, ctx->host);
    ctx->server_ref = DemoServer_new(ctx->port, ctx->host, listening_socket_fd, NULL);
    g_sref = ctx->server_ref;
    DemoServer_listen(ctx->server_ref);
    runloop_run(ctx->server_ref->runloop_ref, -1 /* infinite*/);
    DemoServer_dispose(&ctx->server_ref);
    return NULL;
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
