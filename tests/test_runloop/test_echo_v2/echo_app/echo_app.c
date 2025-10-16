#include "echo_app.h"
NewLineMsgRef process_input_message(NewLineMsgRef input_msg);
static void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd);

EchoAppRef echo_app_new(RunloopRef rl, int connection_fd)
{
    EchoAppRef appref = malloc(sizeof(EchoApp));
    echo_app_init(appref, rl, connection_fd);
    return appref;
}
void echo_app_init(EchoAppRef app, RunloopRef rl, int connection_fd)
{
    RBL_SET_TAG(EchoApp_TAG, app)
    RBL_SET_END_TAG(EchoApp_TAG, app)
    app->msg_stream_ref = msg_stream_new(rl, connection_fd);
    // other stuff to come
}

void echo_app_free(EchoAppRef app)
{
    msg_stream_free(app->msg_stream_ref);
    free(app);
}
void* generic_echo_app_new(RunloopRef rl, int fd)
{
    return echo_app_new(rl, fd);
}
void generic_echo_app_free(void* app)
{
    echo_app_free(app);
}
void generic_echo_app_run(void* app_ref, void(cb)(void* app, void* server, int error), void* arg)
{
    echo_app_run(app_ref, cb, arg);
}


static void msg_read_callback(void* arg, NewLineMsgRef msg, int error);
static void msg_write_callback(void* arg, int error);
static void postable_read(RunloopRef rl, void* arg);
static void invoke_done_callback(EchoAppRef app, int error);

void echo_app_run(EchoAppRef app, AppDoneCallback* cb, void* arg)
{
    app->done_cb = cb;
    app->done_arg = arg;
    RunloopRef rl = msg_stream_get_runloop(app->msg_stream_ref);
    runloop_post(rl, postable_read, app);
}
static void msg_read_callback(void* arg, NewLineMsgRef msg, int error)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    if(error != 0) {
        printf("msg_read_callback error %d  %s\n ", error, strerror(error));
        invoke_done_callback(app, error);
    } else {
        assert(msg != NULL);
        NewLineMsgRef response = process_input_message(msg);
        msg_stream_write(app->msg_stream_ref, response, msg_write_callback, app);
    }
}
static void msg_write_callback(void* arg, int error)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    if(error != 0) {

    }
    RunloopRef rl = msg_stream_get_runloop(app->msg_stream_ref);
    runloop_post(rl, postable_read, app);
}
static void postable_read(RunloopRef rl, void* arg)
{
    EchoAppRef app = arg;
    RBL_CHECK_TAG(EchoApp_TAG, app)
    RBL_CHECK_END_TAG(EchoApp_TAG, app)
    msg_stream_read(app->msg_stream_ref, msg_read_callback, app);
}

NewLineMsgRef process_input_message(NewLineMsgRef input_msg)
{
    NewLineMsgRef response = newline_msg_new();
    char* p = IOBuffer_data(newline_msg_get_content(input_msg));
    int n = IOBuffer_data_len(newline_msg_get_content(input_msg));
    // for (int i = 0; i < n; i++) {
    //     if (*p == '\n') {
    //         *p = 'X';
    //     }
    // }
    char* response_buf_ptr;
    // asprintf(&response_buf_ptr, "ServerResponse:[%s]\n", p);
    asprintf(&response_buf_ptr, "ServerResponse:[%s]\n", p);
    newline_msg_set_content(response, IOBuffer_from_cstring(response_buf_ptr));
    free(response_buf_ptr);
    return response;
}
static void invoke_done_callback(EchoAppRef app, int error)
{
    AppDoneCallback* cb = app->done_cb;
    void* arg = app->done_arg;
    app->done_cb = NULL;
    app->done_arg = NULL;
    cb(app, arg, error);
}