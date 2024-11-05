
#include <http_in_c/async/async.h>
#include <rbl/logger.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>
#include <http_in_c/async/junk/async_internal.h>

// forward decleration of the function that processes a request into a response
static void process_request(AsyncHandlerRef href, MessageRef request);
static void usage();
void sig_handler(int signo);
void* thread_function(void* arg);

#define MAX_NBR_THREADS 20
AsyncServerRef g_sref;
/**
 * Top level structure for keeping track of thread specific structures.
 * TODO: reduce the use of malloc()
 */
typedef struct thread_context_s {
    int             ident;
    pthread_t       thread;
    int             port;
    const char*     host;
    int             listening_socket;
    void*           return_value;
    AsyncServerRef  server_ptr;
} thread_context_t;

static thread_context_t thread_table[MAX_NBR_THREADS];

int main(int argc, char* argv[]) {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    printf("Hello this is xr-junk main \n");
    opterr = 0;
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
                usage();
                exit(0);
        }
    }
#ifdef CH_ASYNC_SINGLE_THREAD
    AsyncServerRef server_ref = AsyncServer_new(port_number, "127.0.0.1", &process_request);
    AsyncServer_start(server_ref);
    async_socket_listen(server_ref->listening_socket_fd);
    runloop_run(server_ref->reactor_ref, -1 /*infinite */);
    AsyncServer_dispose(&server_ref);
#else
    if (nbr_threads > MAX_NBR_THREADS) {
        printf("-t parameter is limited by VERIFY_MAX_THREADS  %d\n", MAX_NBR_THREADS);
        exit(-1);
    }
    int t;
    int listen_socket = async_create_shareable_socket();
    async_socket_bind(listen_socket, port_number, "127.0.0.1");
    async_socket_listen(listen_socket);

    for (t = 0; t < nbr_threads; t++) {
        thread_context_t *ctx = &thread_table[t];
        ctx->ident = t;
        ctx->listening_socket = listen_socket;
        ctx->port = port_number;
        ctx->host = "127.0.0.1";
        int rc = pthread_create(&(ctx->thread), NULL, &thread_function, ctx);
    }
    for (t = 0; t < nbr_threads; t++) {
        thread_context_t *ctx = &thread_table[t];
        pthread_join(ctx->thread, &(ctx->return_value));
    }
    for (t = 0; t < nbr_threads; t++) {
        thread_context_t *ctx = &thread_table[t];
        AsyncServer_dispose(&(ctx->server_ptr));
    }
#endif
}
#
/**
 * The function that does the work of a single thread. No loops because it is
 * event driven. Look inside the AsyncServer_start() function to see how it starts.
 */
void* thread_function(void* arg)
{
    thread_context_t* ctx = arg;
    RBL_LOG_FMT("thread function ident: %d pthread_t: %lu listening_socket: %d", ctx->ident, ctx->thread, ctx->listening_socket)
    ctx->server_ptr = AsyncServer_new_with_socket(ctx->port, ctx->host, ctx->listening_socket, process_request);
    g_sref = ctx->server_ptr;
    AsyncServer_start(ctx->server_ptr);
    async_socket_listen(ctx->server_ptr->listening_socket_fd);
    runloop_run(ctx->server_ptr->reactor_ref, -1 /* infinite*/);
    AsyncServer_dispose(&ctx->server_ptr);
}
void sig_handler(int signo)
{
    printf("async_server_main.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        // TODO fix this up
        return;
        AsyncServer_free(g_sref);
        g_sref = NULL;
        exit(0);
    }
}
/**
 * From this point down is the code that handles a request and makes the appropriate response
 */

static BufferChainRef simple_response_body(char* message, int socket, int pthread_self_value);
static char* echo_body(MessageRef request);
static BufferChainRef echo_body_buffer_chain(MessageRef request);
static MessageRef app_handler_example(AsyncHandlerRef handler_ref, MessageRef request);

static void process_request(AsyncHandlerRef href, MessageRef request)
{
    RBL_CHECK_TAG(AsyncHandler_TAG, href)
    RBL_LOG_FMT("request %p", request)
    IOBufferRef iob = Message_dump(request);
    MessageRef reply = app_handler_example(href, request);
    IOBufferRef iobreply = Message_dump(reply);

    IOBuffer_free(iobreply);
    IOBuffer_free(iob);
    href->handle_response(href, request, reply);
    return;

}

static BufferChainRef simple_response_body(char* message, int socket, int pthread_self_value)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char dt_s[64];
    assert(strftime(dt_s, sizeof(dt_s), "%c", tm));
    BufferChainRef bchain = BufferChain_new();
    char* body = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "%s"
                 "<p>Date/Time is %s</p>"
                 "<p>socket: %d</p>"
                 "<p>p_thread_self %ld</p>"
                 "</body>"
                 "</html>";

    char* s1;
    int len1 = asprintf(&s1, body, message, dt_s, socket, pthread_self_value);
    BufferChain_append_cstr(bchain, s1);
    free(s1);
    return bchain;
}
static char* echo_body(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    char* body = malloc(IOBuffer_data_len(iob_body) + 1);
    memcpy(body, IOBuffer_data(iob_body), IOBuffer_data_len(iob_body)+1);
    return body;
}

static BufferChainRef echo_body_buffer_chain(MessageRef request)
{
    IOBufferRef iob_body = Message_serialize(request);
    BufferChainRef bufchain = BufferChain_new();
    BufferChain_append_IOBuffer(bufchain, iob_body);
    return bufchain;
}
static MessageRef app_handler_example(AsyncHandlerRef handler_ref, MessageRef request)
{
    MessageRef response = Message_new_response();
    char* msg = "<h2>this is a message</h2>";
    BufferChainRef body_chain = NULL;
    char* body = NULL;
    char* body_len_str = NULL;
    int return_value = 0;

    CbufferRef target = Message_get_target_cbuffer(request);
    const char* target_cstr = Cbuffer_cstr(target);
    if(strcmp(target_cstr, "/echo") == 0) {
        /**
         * find the echo-id header in the request and put it in the response headers
         */
        const char* echo_value = Message_get_header_value(request, HEADER_ECHO_ID);
        if(echo_value != NULL) {
            Message_add_header_cstring(response, HEADER_ECHO_ID, echo_value);
        }
        body_chain = echo_body_buffer_chain(request);
    } else {
        int s = handler_ref->async_connection_ref->socket;
        int t = async_handler_threadid(handler_ref);
        body_chain = simple_response_body(msg, s, t);
    }
    Message_set_status(response, HTTP_STATUS_OK);
    Message_set_reason(response, "OK");
    Message_add_header_cstring(response, HEADER_CONTENT_TYPE, "text/html; charset=UTF-8");

    Message_set_body(response, body_chain);
    Message_set_content_length(response, BufferChain_size(body_chain));

    IOBufferRef request_serialized = Message_serialize(request);
    IOBufferRef response_serialized = Message_serialize(response);
//    printf("app_handler_example request : %s\n", IOBuffer_cstr(request_serialized));
//    printf("app_handler_example response : %s\n", IOBuffer_cstr(response_serialized));
    IOBuffer_free(request_serialized);
    IOBuffer_free(response_serialized);
    return response;
}
static void usage()
{
    printf("Name: sync_server\n");
    printf("\nDescription\n");
    printf("\tasync_server is a multi-threaded event based http server. \n");
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