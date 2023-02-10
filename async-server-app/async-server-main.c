
#include <http_in_c/async/async.h>
#include <http_in_c/logger.h>
#include <stdio.h>
#include <mcheck.h>
#include<signal.h>

// forward decleration of the function that processes a request into a response
static MessageRef process_request(AsyncHandlerRef href, MessageRef request);

AsyncServerRef g_sref;
void sig_handler(int signo)
{
    printf("demo_app.c signal handler \n");
    if ((signo == SIGINT) || (signo == SIGABRT)) {
        printf("received SIGINT or SIGABRT\n");
        AsyncServer_free(g_sref);
        g_sref = NULL;
        exit(0);
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    if (signal(SIGABRT, sig_handler) == SIG_ERR) {
        printf("sync_app.c main signal() failed");
    }
    printf("Hello this is xr-junk main \n");
    AsyncServerRef sref = AsyncServer_new(9001, process_request);
    g_sref = sref;
    AsyncServer_listen(sref);
    AsyncServer_dispose(&sref);
}

static BufferChainRef simple_response_body(char* message, int socket, int pthread_self_value);
static char* echo_body(MessageRef request);
static BufferChainRef echo_body_buffer_chain(MessageRef request);
static MessageRef app_handler_example(AsyncHandlerRef handler_ref, MessageRef request);

static MessageRef process_request(AsyncHandlerRef href, MessageRef request)
{
    CHECK_TAG(AsyncHandler_TAG, href)
    LOG_FMT("request %p", request)
    IOBufferRef iob = Message_dump(request);
    MessageRef reply = app_handler_example(href, request);
    IOBufferRef iobreply = Message_dump(reply);
    IOBuffer_free(iobreply);
    IOBuffer_free(iob);
    return reply;
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
